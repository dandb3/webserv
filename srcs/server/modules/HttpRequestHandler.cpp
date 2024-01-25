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
    size_t end;

    if ((end = _remain.find(CRLF)) == std::string::npos)
        return;

    _lineV.push_back(_remain.substr(0, end));
    _remain = _remain.substr(end + 2);
    _parseRequestLine();
}

void HttpRequestHandler::_parseQuery(RequestLine &requestLine, std::string &query)
{
    std::vector<std::pair<std::string, std::string> > queryV;
    std::string key, value;
    size_t equalPos, amperPos, start = 0;

    while (start != query.length()) {
        if ((equalPos = key.find('=', start + 1)) == std::string::npos)
            return; // GET 요청에 문법 오류(error 발생)

        if ((amperPos = query.find('&', equalPos + 1)) == std::string::npos)
            amperPos = query.length();

        key = query.substr(start + 1, equalPos - start - 1);
        value = query.substr(equalPos + 1, amperPos - equalPos - 1);
        queryV.push_back(std::make_pair(key, value));

        start = amperPos;
    }

    requestLine.setQuery(queryV);
}

std::string HttpRequestHandler::_decodeUrl(std::string &str)
{
    std::ostringstream decoded;

    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%') {
            if (i + 2 < str.length() && isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
                char decodedChar = static_cast<char>(std::strtol(str.substr(i + 1, 2), nullptr, 16));
                decoded << decodedChar;
                i += 2;
            }
            else
                decoded << str[i];
        }
        else if (str[i] == '+')
            decoded << ' ';
        else
            decoded << str[i];
    }

    return decoded.str();
}

bool HttpRequestHandler::_parseRequestLine()
{
    std::string requestLineStr = _lineV[0];

    std::istringstream iss(requestLineStr);
    std::vector<std::string> tokens;

    std::string token;
    while (iss >> token)
        tokens.push_back(token);

    if (tokens.size() != 3) // 400 ERROR
        return FAILURE;

    RequestLine requestLine;

    // set method
    token = tokens[0];
    short method;
    if (token == "GET")
        method = GET;
    else if (token == "HEAD")
        method = HEAD;
    else if (token == "POST")
        method = POST;
    else if (token == "DELETE")
        method = DELETE;
    else // 처리하지 않을 header -> 501 Not Implemented
        return FAILURE;
    requestLine.setMethod(method);

    // set uri & query
    // uri가 긴 경우 -> 414 (URI too long) (8000 octets 넘어가는 경우)
    token = decodeUrl(tokens[1]);
    std::vector<std::pair<std::string, std::string> > queryV;
    size_t pos = token.find('?');
    if (pos == std::string::npos) 
        requestLine.setUri(token);
    else {
        std::string uri = token.substr(0, pos);
        std::string queries = token.substr(pos + 1);
        requestLine.setUri(uri);
        _parseQuery(requestLine, queries);
    }

    // set HTTP version
    token = tokens[2];
    if (token.substr(0, 5) != "HTTP/" || token.length() != 8) // 정의되어있지 않음(내가 못찾은 거일수도) -> 400 error?
        return FAILURE;

    requestLine.setVersion(std::make_pair(static_cast<short>(token[5] - '0'), static_cast<short>(token[7] - '0')));

    _httpRequest.setRequestLine(requestLine);
    _status = INPUT_HEADER_FIELD;

    return SUCCESS;
}

void HttpRequestHandler::_inputHeaderField()
{
    const std::string whitespace = WHITESPACE;
    size_t start, end;
    bool lastWhitespace;
    bool crlfFound = false;

    start = 0;
    _lineV.clear();
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        if (start == end)
            break;
        lastWhitespace = (whitespace.find(_remain[end - 1]) != std::string::npos);
        crlfFound = true;
        _lineV.push_back(_remain.substr(start, end - start + 1 - lastWhitespace));
        start = end + 2;
    }
    if (crlfFound)
        _remain = _remain.substr(start + 2);
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
        pos = pos + 1 + (whitespace.find(_lineV[i][pos + 1]) != std::string::npos);
        value = _lineV[i].substr(pos, _lineV[i].length() - pos);
        headerFields.insert(make_pair(key, value));
    }

    _httpRequest.setHeaderFields(headerFields);
    _status = INPUT_MESSAGE_BODY;
    return SUCCESS;
}

/*
Case of the messsage body
1. content-length O, transfer-encoding X -> message body 그대로
2. content-length X, transfer-encoding X -> request message가 payload body를 가지고 있지 않고, method가 body에 의미를 두지 않은 경우
3. content-length X, transfer-encoding: chunked -> chunked 처리
4. content-length X, transfer-encoding: gzip -> 에러로 간주하기
5. content-length O, transfer-encoding O -> sender MUST NOT
6. content-length가 여러개 있거나 list형태로 오는 경우
*/
void HttpRequestHandler::_inputMessageBody()
{
    int contentLengthCount = _httpRequest.getHeaderFields().count("Content-Length");
    int transferEncodingCount = _httpRequest.getHeaderFields().count("Transfer-Encoding");

    if (contentLengthCount > 0)
        _inputDefaultBody(contentLengthCount, transferEncodingCount);
    else if (transferEncodingCount > 0)
        _inputChunkedBody(transferEncodingCount);
    else
        _status = PARSE_FINISHED;
}

void HttpRequestHandler::_inputDefaultBody(int contentLengthCount, int transferEncodingCount)
{
    std::string lengthStr;
    long long length;

    if (transferEncodingCount > 0) // sender MUST NOT
        return;
    if (contentLengthCount == 1)
        lengthStr = _httpRequest.getHeaderFields().find("Content-Length")->second;
    else {
        std::multimap<std::string, std::string> &mp = _httpRequest.getHeaderFields();
        std::multimap<std::string, std::string>::iterator iter = mp.find("Content-Length");

        lengthStr = iter->second;
        for (iter++; iter != mp.end() && iter->first == "Content-Length"; iter++) {
            if (lengthStr != iter->second) // 404 error
                return;
        }
    }

    std::vector<std::string> lengthV = _splitByComma(lengthStr);
    if (lengthV.size() != 1) {
        for (size_t i = 1; i < lengthV.size(); i++) {
            if (lengthV[0] != lengthV[i]) // 404 error
                return;
        }
        lengthStr = lengthV[0];
    }
    length = strtol(lengthStr.c_str(), NULL, 10);

    _httpRequest.getMessageBody().append(_remain.substr(0, length));
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
    short mode = LENGTH;

    if (_httpRequest.getHeaderFields().find("Transfer-Encoding")->second != "chunked") // chunked가 아님 -> 400 ERROR
        return;

    start = 0;
    _lineV.clear();
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        if (mode == LENGTH) {
            length = strtol(_remain.substr(start, end - start).c_str(), NULL, 10);
            mode = STRING;
        }
        else {
            _httpRequest.getMessageBody().append(_remain.substr(start, length));
            mode = LENGTH;
        }
        start = end + 2;
    }
    _httpRequest.getMessageBody().append("\0");
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
            throw std::runtime_error("recvHttpRequest에서 read 실패");
        size -= read_len;
        _remain.append(_buf, static_cast<size_t>(read_len));
    }
    // configuration의 client-body size도 고려해야 함
    // if ((read_len = read(fd, _buf, std::min(size, static_cast<size_t>(BUF_SIZE)))) == FAILURE) {
    //     throw std::runtime_error("recvHttpRequest에서 read 실패");
    // }
    // size -= read_len;
    // _remain.append(_buf, static_cast<size_t>(read_len));
}

void HttpRequestHandler::parseHttpRequest(bool eof, std::queue<HttpRequest> &httpRequestQ)
{
    do {
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

std::vector<std::string> HttpRequestHandler::_splitByComma(std::string &str)
{
    std::vector<std::string> ret;
    std::string token;
    std::istringstream iss(str);

    while (std::getline(iss, token, ',')) {
        if (token[0] == ' ')
            ret.push_back(token.substr(1));
        else
            ret.push_back(token);
    }
    return ret;
}