#include <sstream>
#include "HttpRequestModule.hpp"

HttpRequestHandler::HttpRequestHandler() : _status(INPUT_READY)
{}

void HttpRequestHandler::_inputStart()
{
    _status = INPUT_REQUEST_LINE;
}

void HttpRequestHandler::_inputRequestLine()
{
    size_t start, end;

    start = 0;
    if ((end = _remain.find(CRLF, start)) == std::string::npos)
        return;

    _lineV.push_back(_remain.substr(start, end));
    _remain = _remain.substr(end + 2);
    _parseRequestLine();
}

bool HttpRequestHandler::_parseRequestLine()
{
    std::string requestLineStr = _lineV[0];

    std::istringstream iss(requestLineStr);
    std::vector<std::string> tokens;

    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() != 3) // 400 ERROR
        return FAILURE;

    RequestLine requestLine;

    // set method
    token = tokens[0];
    if (token == "GET")
        requestLine.setMethod(GET);
    else if (token == "HEAD")
        requestLine.setMethod(HEAD);
    else if (token == "POST")
        requestLine.setMethod(POST);
    else if (token == "DELETE")
        requestLine.setMethod(DELETE);
    else // 처리하지 않을 header -> 501 Not Implemented
        return FAILURE;

    // set uri
    requestLine.setRequestTarget(tokens[1]);

    // set HTTP version
    token = tokens[2];
    if (token.substr(0, 5) != "HTTP/" || token.length() != 8)
        return FAILURE;

    requestLine.setVersion(std::make_pair(atoi(token[5]), atoi(token[7])));

    _httpRequest.setRequestLine(requestLine);
    _status = INPUT_HEADER_FIELD;

    return SUCCESS;
}

void HttpRequestHandler::_inputHeaderField()
{
    const std::string whitespace = WHITESPACE;
    size_t start, end;
    bool crlfFound = false;

    start = 0;
    _lineV.clear();
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        crlfFound = true;

        _lineV.push_back(_remain.substr(start, end - (whitespace.find(_remain[end - 1]) != string::npos))); // remove last OWS
        start = end + 2;

        // if CRLF CRLF comes
        if (_lineV.rbegin()->empty()) {
            _status = PARSE_HEADER_FIELD;
            break;
        }
    }
    if (crlfFound)
        _remain = _remain.substr(start);

    _parseHeaderField();
}

bool HttpRequestHandler::_parseHeaderField()
{
    std::multimap<std::string, std::string> headerFields;
    const std::string whitespace = WHITESPACE;
    std::string key, value;
    size_t pos;

    for (int i = 0; i < _lineV.size(); i++) {
        if ((pos = _lineV[i].find(':')) == std::string::npos)
            return FAILURE; // maybe a obs-fold -> 400 ERROR
        if (whitespace.find(_lineV[i][pos - 1]) != std::string::npos)
            return FAILURE; // 400 ERROR

        key = _lineV[i].substr(0, pos);
        pos = pos + 1 + (whitespace.find(_lineV[i][pos]) != std::string::npos);
        value = _lineV[i].substr(pos, _lineV[i].length() - pos);
        _headerFields.insert(make_pair(key, value));
    }

    _httpRequest.setHeaderFields(headerFields);
    _status = INPUT_MESSAGE_BODY;
    return SUCCESS;
}

/*
Case of the messsage body
1. content-length O, transfer-encoding X -> message body 그대로
2. content-length X, transfer-encoding X -> request message가 payload body를 가지고 있지 않고, method가 body에 의미를 두지 않은 경우
3. content-length X, transfer-encoding: chunked
4. content-length X, transfer-encoding: gzip
5. content-length O, transfer-encoding O -> sender MUST NOT
6. content-length가 여러개 있거나 list형태로 오는 경우
*/
void HttpRequestHandler::_inputMessageBody()
{
    int contentLengthCount = _headerFields.count("Content-Length");
    int transferEncodingCount = _headerFields.count("Transfer-Encoding");

    if (contentLengthCount > 0)
        _inputDefaultBody(contentLengthCount, transferEncodingCount);
    else
        _inputChunkedBody(transferEncodingCount);
}

void HttpRequestHandler::_inputDefaultBody(int contentLengthCount, int transferEncodingCount)
{
    std::string legnthString;
    long long length;

    if (transferEncodingCount > 0) // sender MUST NOT
        return;
    if (contentLengthCount == 1)
        legnthString = _headerFields["Content-Length"];
    else {
        std::multimap<std::string, std::string>::iterator iter = _headerFields.find("Content-Length");
        legnthString = std::strtol(iter->second);
        for (iter++; iter != _headerFields.end() && iter->first == "Content-Length"; iter++) {
            if (legnthString != iter->second)
                return;
        }
    }
    if (legnthString.find(',') != std::string::npos) // content-length가 x, y 형태로 들어온 경우 -> 오류로 취급
        return;
    length = std::strtol(legnthString);

    _message_body += _remain.substr(0, length);
    _status = PARSE_FINISHED;
}

void HttpRequestHandler::_inputChunkedBody(int transferEncodingCount)
{
    enum {
        LENGTH = 0,
        STRING
    };

    size_t start, end;
    long long length;
    bool mode = LENGTH;

    if (transferEncodingCount == 0) {
        _status = PARSE_FINISHED;
        return;
    }

    if (_headerFields["Transfer-Encoding"] != "chunked") { // chunked가 아닌 경우 error로 간주하기
        return;
    }

    start = 0;
    _lineV.clear();
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        if (mode == LENGTH) {
            length = strtol(_remain.substr(start, end));
            mode = STRING;
        }
        else {
            _message_body += _remain.substr(start, length);
            mode = LENGTH;
        }
        start += 2;
    }
    _status = PARSE_FINISHED;
}

void HttpRequestHandler::_push_err_request(std::queue<HttpRequest> &httpRequestQ)
{
    httpRequestQ.push(_httpRequest);
    _status = INPUT_CLOSED;
}

void HttpRequestHandler::_push_request(std::queue<HttpRequest> &httpRequestQ)
{
    httpRequestQ.push(_httpRequest);
    _status = INPUT_READY;
    // 만약 Connection: closed라면 -> _status = INPUT_CLOSED;
}

void HttpRequestHandler::recvHttpRequest(int fd, size_t size)
{
    ssize_t read_len;

    while (size > 0) {
        if ((read_len = read(fd, _buf, std::min(size, static_cast<size_t>(BUF_SIZE)))) == FAILURE)
            throw err_syscall();
        size -= read_len;
        _remain.append(_buf, static_cast<size_t>(read_len));
    }
}

void HttpRequestHandler::parseHttpRequest(bool eof, std::queue<HttpRequest> &httpRequestQ)
{
    do {
        // 사실 여기서 status 값으로 PARSE_* 애들은 빼도 될 것 같음.
        // 그냥 _input_*함수 내부로 집어넣어도 전혀 상관 없을듯.
        if (_status == INPUT_READY)
            _inputStart();
        if (_status == INPUT_REQUEST_LINE)
            _inputRequestLine();
        if (_status == INPUT_HEADER_FIELD)
            _inputHeaderField();
        if (_status == INPUT_MESSAGE_BODY)
            _inputMessageBody();
        if (_status != PARSE_FINISHED && eof)
            _push_err_request(httpRequestQ);
        if (_status == PARSE_FINISHED)
            _push_request(httpRequestQ);
    } while (_status == INPUT_READY);
}
