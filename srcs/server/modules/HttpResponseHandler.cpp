#include "HttpResponseModule.hpp"

void HttpResponseHandler::_makeStatusLine(StatusLine &statusLine, short code)
{
    std::string text;

    statusLine.setVersion(make_pair(1, 1));
    statusLine.setCode(code);

    switch (code) {
        case 100:
            text = "Continue";
            break;
        case 101:
            text = "Switching Protocol";
            break;
        case 102:
            text = "Processing";
            break;
        case 200:
            text = "OK";
            break;
        case 201:
            text = "Created";
            break;
        case 202:
            text = "Accepted";
            break;
        case 203:
            text = "Non-Authoritative Information";
            break;
        case 204:
            text = "No Content";
            break;
        case 404:
            text = "Not Found";
            break;
        case 503:
            text = "Service Unavailable";
            break;
        default:
            text = "Not Yet Setted\n";
    }

    statusLine.setText(text);
}

void HttpResponseHandler::_setFileTime(std::multimap<std::string, std::string> &headerFields, const char *path)
{
    struct stat fileInfo;
    char buffer[100];
    
    if (path == "")
        return;

    stat(path, &fileInfo);
    std::time_t lastModifiedTime = fileInfo.st_mtime;
    std::tm* timeInfo = std::gmtime(&lastModifiedTime);
    
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    headerFields["Last-Modified"] = std::string(buffer);
}

void HttpResponseHandler::_setDate(std::multimap<std::string, std::string> &headerFields)
{
    std::time_t currentDate = std::time(nullptr);
    std::tm* timeInfo = std::gmtime(&currentDate);
    char buffer[100];

    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    headerFields["Date"] = std::string(buffer);
}

// 수정 필요
void HttpResponseHandler::_setContentType(std::multimap<std::string, std::string> &headerFields)
{
    headerFields["Content-Type"] = "text/html";
}

// 수정 필요
void HttpResponseHandler::_setContentLength(std::multimap<std::string, std::string> &headerFields)
{
    //headerFields["Content-Length"] = "0";
}

// 수정 필요
void HttpResponseHandler::_setConnection(std::multimap<std::string, std::string> &headerFields)
{
    if (1)
        headerFields["Connection"] = "keep-alive";
    else    
        headerFields["Connection"] = "close";

}

void HttpResponseHandler::_makeHeaderFields(std::multimap<std::string, std::string> &headerFields, NetConfig &netConfig)
{
    _setDate(headerFields);
    _setContentType(headerFields);
    _setConnection(headerFields);
    // _setContentLength(headerFields);
    // _setFileTime(headerFields, netConfig.getPath());
}

void HttpResponseHandler::_makeGETResponse(HttpRequest &httpRequest, NetConfig &netConfig, bool isGET)
{
    int fileFd = open(netConfig.getPath());
    StatusLine statusLine;
    std::multimap<std::string, std::string> headerFields;
    std::string messageBody;

    if (fileFd == -1) {
        _makeStatusLine(statusLine, 404);
        fildFd = open(netConfig.getErrorPath());

        // 404 Not Found 페이지가 없는 경우
        if (fileFd == -1)
            throw err_syscall();
    }
    else {
        _makeStatusLine(statusLine, 200);
    }

    // fileFd로부터 해당 파일을 읽어온다.
    // std::string에 append 혹은 push_back을 통해서 body를 만든다.(C++ version 확인)
    // HTTPresponse의 messagebody에 할당해준다.
    // method가 HEAD인 경우에 body를 세팅하지 않는다.
    if (isGET) {
        char buffer[1024];
        while (read(fileFd, buffer, 1024) > 0)
            messageBody.append(buffer);
    }

    // Header Field들을 세팅해준다.
    _makeHeaderFields(headerFields);

    // chunked?
    headerFields["Content-Length"] = toString(messageBody.length());

    _httpResponse.setStatusLine(statusLine);
    _httpResponse.setHeaderFields(headerFields);
    _httpResponse.setMessageBody(messageBody);

    close(fileFd);
}

void HttpResponseHandler::_makeHEADResponse(HttpRequest &httpRequest, NetConfig &netConfig)
{
    (void)httpRequest;
    (void)netConfig;
}

void HttpResponseHandler::_makePUTResponse(HttpRequest &httpRequest, NetConfig &netConfig)
{
    (void)httpRequest;
    (void)netConfig;
}

void HttpResponseHandler::_makeDELETEResponse(HttpRequest &httpRequest, NetConfig &netConfig)
{
    (void)httpRequest;
    (void)netConfig;
}


void HttpResponseHandler::_statusLineToString()
{
    const std::pair<short, short> version = _httpResponse.getStatusLine().getVersion();
    std::string versionStr;
    std::string codeStr;

    versionStr = "HTTP/";
    versionStr.push_back(static_cast<char>(version.first + '0'));
    versionStr.push_back('.');
    versionStr.push_back(static_cast<char>(version.second + '0'));

    codeStr = toString(_httpResponse.getStatusLine().getCode());

    _response = versionStr + " " + codeStr + " " + _httpResponse.getStatusLine().getText() + CRLF;
}

void HttpResponseHandler::_headerFieldsToString(std::multimap<std::string, std::string> &headerFields)
{
    std::multimap<std::string, std::string>::iterator it;
    for (it = headerFields.begin(); it != headerFields.end(); it++) {
        _response += it->first + ": " + it->second + CRLF;
    }
    _response += CRLF;
}

void HttpResponseHandler::_httpResponseToString()
{
    _statusLineToString();
    _headerFieldsToString();
    _response += _httpResponse.getMessageBody();
}

void HttpResponseHandler::makeHttpResponse(HttpRequest &httpRequest, NetConfig &netConfig)
{
    const short method = httpRequest.getRequestLine().getMethod();

    // if else -> switch?
    if (method == GET || method == HEAD) {
        _makeGETResponse(httpRequest, netConfig, (method == GET));
    }
    else if (method == PUT) {
        _makePUTResponse();
    }
    else if (method == DELETE) {
        _makeDELETEResponse();
    }
    else {
        cout << "??";
    }

    _httpResponseToString();
}

void HttpResponseHandler::sendHttpResponse(int fd, size_t size)
{
    size_t writeLen;

    writeLen = std::min(_response.size() - _pos, size);
    if (write(fd, _response.c_str() + _pos, writeLen) == FAILURE)
        throw err_syscall();
    _pos += writeLen;
    if (_pos == size) {
        _status = RES_IDLE;
        _pos = 0;
    }
}