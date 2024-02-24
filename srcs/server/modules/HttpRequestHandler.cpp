#include <sstream>
#include "HttpRequestModule.hpp"
#include "../parse/parse.hpp"
#include "../../utils/utils.hpp"

HttpRequestHandler::HttpRequestHandler()
    : _status(INPUT_READY)
{}

void HttpRequestHandler::_inputEOF()
{
    if (_status == INPUT_READY || _status == INPUT_START)
        _status = INPUT_NORMAL_CLOSED;
    else
        _status = INPUT_ERROR_CLOSED;
}

void HttpRequestHandler::_inputReady()
{
    _status = INPUT_START;
}

void HttpRequestHandler::_inputStart()
{
    if (!_remain.empty())
        _status = INPUT_REQUEST_LINE;
}

void HttpRequestHandler::_inputRequestLine()
{
    size_t end;

    // 아직 CRLF가 안나온 경우
    if ((end = _remain.find(CRLF)) == std::string::npos)
        return;

    _lineV.clear();
    _lineV.push_back(_remain.substr(0, end));
    _remain = _remain.substr(end + 2);
    _parseRequestLine();
}

void HttpRequestHandler::_parseRequestLine()
{
    std::string requestLineStr = _lineV[0];
    _lineV.clear();

    std::istringstream iss(requestLineStr);
    std::vector<std::string> tokens;

    std::string token;
    while (iss >> token)
        tokens.push_back(token);

    if (tokens.size() != 3) {
        _status = INPUT_ERROR_CLOSED;
        _httpRequest.setCode(400);
        return;
    }

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
    else { // 처리하지 않을 header -> 501 Not Implemented
        _httpRequest.setCode(501);
        method = METHOD_ERROR;
    }
    requestLine.setMethod(method);

    // set uri & query
    token = decodeUrl(tokens[1]);
    if (token.length() > 8000) { // uri가 긴 경우 -> 414 (URI too long) (8000 octets 넘어가는 경우)
        _httpRequest.setCode(414);
        return;
    }

    std::vector<std::pair<std::string, std::string> > queryV;
    size_t qPos = token.find('?'), fPos = token.find('#');
    if (qPos == std::string::npos) {
        if (fPos == std::string::npos)
            requestLine.setUri(token);
        else
            requestLine.setUri(token.substr(0, fPos));
    }
    else {
        std::vector<std::pair<std::string, std::string> > queryV;
        std::string queries;

        requestLine.setUri(token.substr(0, qPos));
        if (fPos == std::string::npos)
            queries = token.substr(qPos + 1);
        else
            queries = token.substr(qPos + 1, fPos - qPos - 1);
        queryV = parseQuery(queries);
        if (queryV.empty()) {
            _status = INPUT_ERROR_CLOSED;
            _httpRequest.setCode(400);
            return;
        }
    }
    if (fPos != std::string::npos)
        requestLine.setFragment(token.substr(fPos + 1));
    
    // set HTTP version
    token = tokens[2];
    if (token.substr(0, 5) != "HTTP/" || token.length() != 8 || token[6] != '.') { // 정의되어있지 않음 -> 400 error
        _httpRequest.setCode(400);
        return;
    }

    short first = static_cast<short>(token[5] - '0');
    short second = static_cast<short>(token[7] - '0');
    if (first != 1 || second != 1)
        _httpRequest.setCode(505); // HTTP version not supported (1.1만 지원)
    requestLine.setVersion(std::make_pair(first, second));
    
    _httpRequest.setRequestLine(requestLine);
    _status = INPUT_HEADER_FIELD;
}

void HttpRequestHandler::_inputByLine(bool isHeaderField)
{
    const std::string whitespace(WHITESPACE);
    size_t start, end;
    bool lastWhitespace, lastCRLF;

    lastCRLF = false;
    start = 0;
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        if (start == end) {
            lastCRLF = true;
            _remain = _remain.substr(2);
            break;
        }
        lastWhitespace = (whitespace.find(_remain[end - 1]) != std::string::npos);
        _lineV.push_back(_remain.substr(start, end - start - lastWhitespace));
        start = end + 2;
    }

    if (start != 0)
        _remain = _remain.substr(start);
    if (lastCRLF) {
        if (isHeaderField)
            _parseHeaderField();
        else
            _parseChunkedBody();
    }
}

void HttpRequestHandler::_parseHeaderField()
{
    std::multimap<std::string, std::string> headerFields;
    const std::string whitespace = WHITESPACE;
    std::string key, value;
    size_t pos;

    for (size_t i = 0; i < _lineV.size(); i++) {
        if ((pos = _lineV[i].find(':')) == std::string::npos) { // maybe a obs-fold -> 400 ERROR
            _status = INPUT_ERROR_CLOSED;
            _httpRequest.setCode(400);
            return;
        }
        if (whitespace.find(_lineV[i][pos - 1]) != std::string::npos) {
            _status = INPUT_ERROR_CLOSED;
            _httpRequest.setCode(400);
            return;
        }

        key = _lineV[i].substr(0, pos);
        pos = pos + 1 + (whitespace.find(_lineV[i][pos + 1]) != std::string::npos);
        value = _lineV[i].substr(pos, _lineV[i].length() - pos);
        headerFields.insert(make_pair(key, value));
    }

    _lineV.clear();
    _httpRequest.setHeaderFields(headerFields);
    
    _status = INPUT_MESSAGE_BODY;
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

    if (contentLengthCount > 0) {
        if (transferEncodingCount > 0) {
            _status = INPUT_ERROR_CLOSED;
            _httpRequest.setCode(400);
            return;
        }
        _extractContentLength(contentLengthCount);
        _status = INPUT_DEFAULT_BODY;
        _inputDefaultBody();
    }
    else if (transferEncodingCount > 0) {
        if (_httpRequest.getHeaderFields().find("Transfer-Encoding")->second != "chunked") { // chunked가 아님 -> 400 ERROR
            _status = INPUT_ERROR_CLOSED;
            _httpRequest.setCode(400);
            return;
        }
        _lineV.clear();
        _status = INPUT_CHUNKED_BODY;
        _inputByLine(false);
    }
    else
        _status = PARSE_FINISHED;
}

void HttpRequestHandler::_extractContentLength(int contentLengthCount)
{
    std::string lengthStr;

    if (contentLengthCount == 1)
        lengthStr = _httpRequest.getHeaderFields().find("Content-Length")->second;
    else {
        std::multimap<std::string, std::string> &mp = _httpRequest.getHeaderFields();
        std::multimap<std::string, std::string>::iterator iter = mp.find("Content-Length");

        lengthStr = iter->second;
        for (iter++; iter != mp.end() && iter->first == "Content-Length"; iter++) {
            if (lengthStr != iter->second) { // 400 error
                _status = INPUT_ERROR_CLOSED;
                _httpRequest.setCode(400);
                return;
            }
        }
    }

    std::vector<std::string> lengthV = splitByDlm(lengthStr, ',');
    if (lengthV.size() != 1) {
        for (size_t i = 1; i < lengthV.size(); i++) {
            if (lengthV[0] != lengthV[i]) { // 400 error
                _status = INPUT_ERROR_CLOSED;
                _httpRequest.setCode(400);
                return;
            }
        }
        lengthStr = lengthV[0];
    }
    _contentLength = strtol(lengthStr.c_str(), NULL, 10);
}

void HttpRequestHandler::_inputDefaultBody()
{
    if (_remain.length() < _contentLength) {
        _httpRequest.getMessageBody().append(_remain);
        _remain = "";
        _contentLength -= _remain.length();
    }
    else { // remain에 다음 request가 포함되어 있을 수 있다.
        _httpRequest.getMessageBody().append(_remain.substr(0, _contentLength));
        _remain = _remain.substr(_contentLength);
        _contentLength = 0;
        _status = PARSE_FINISHED;
    }
}

void HttpRequestHandler::_parseChunkedBody()
{
    long long length;

    if (!(_lineV.size() & 1) || _lineV.back() != "0") {
        _status = INPUT_ERROR_CLOSED;
        _httpRequest.setCode(400);
        return;
    }
    for (size_t i = 0; i < _lineV.size() - 1; i++) {
        if (!(i & 1)) {
            length = strtol(_lineV[i].c_str(), NULL, 16);
        }
        else {
            if (length != static_cast<long long>(_lineV[i].length())) {
                _status = INPUT_ERROR_CLOSED;
                _httpRequest.setCode(400);
                return;
            }
            _httpRequest.getMessageBody().append(_lineV[i]);
        }
    }
    _lineV.clear();
    _status = PARSE_FINISHED;
}

void HttpRequestHandler::_pushRequest(std::queue<HttpRequest> &httpRequestQ)
{
    const std::multimap<std::string, std::string>& headerFields = _httpRequest.getHeaderFields();
    std::multimap<std::string, std::string>::const_iterator it;

    httpRequestQ.push(_httpRequest);

    if (_status != INPUT_ERROR_CLOSED) {
        it = headerFields.find("Connection");
        if (it != headerFields.end() && isCaseInsensitiveSame(it->second, "close"))
            _status = INPUT_NORMAL_CLOSED;
        else
            _status = INPUT_READY;
    }
    _httpRequest = HttpRequest();
}

void HttpRequestHandler::recvHttpRequest(int fd, size_t size)
{
    ssize_t readLen;

    if ((readLen = read(fd, ICycle::getBuf(), std::min(size, static_cast<size_t>(BUF_SIZE)))) == FAILURE) {
        _httpRequest.setCode(500);
        _status = INPUT_ERROR_CLOSED;
        return;
    }
    _remain.append(ICycle::getBuf(), static_cast<size_t>(readLen));
}

void HttpRequestHandler::parseHttpRequest(bool eof, std::queue<HttpRequest> &httpRequestQ)
{
    if (eof)
        _inputEOF();
    do {
        if (_status == INPUT_READY)
            _inputReady();
        if (_status == INPUT_START)
            _inputStart();
        if (_status == INPUT_REQUEST_LINE)
            _inputRequestLine();
        if (_status == INPUT_HEADER_FIELD)
            _inputByLine(true);
        if (_status == INPUT_MESSAGE_BODY)
            _inputMessageBody();
        if (_status == INPUT_DEFAULT_BODY)
            _inputDefaultBody();
        if (_status == INPUT_CHUNKED_BODY)
            _inputByLine(false);
        if (_status == PARSE_FINISHED || _status == INPUT_ERROR_CLOSED)
            _pushRequest(httpRequestQ);
    } while (_status == INPUT_READY);
}

HttpRequest& HttpRequestHandler::getHttpRequest()
{
    return _cycleHttpRequest;
}
void HttpRequestHandler::setHttpRequest(const HttpRequest& httpRequest)
{
    _cycleHttpRequest = httpRequest;
}

bool HttpRequestHandler::isInputReady() const
{
    return (_status == INPUT_READY || _status == INPUT_START);
}

bool HttpRequestHandler::closed() const
{
    return (_status == INPUT_NORMAL_CLOSED || _status == INPUT_ERROR_CLOSED);
}

void HttpRequestHandler::reset()
{
    _cycleHttpRequest = HttpRequest();
}
