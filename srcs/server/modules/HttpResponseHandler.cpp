#include "HttpResponseModule.hpp"

HttpResponseHandler::HttpResponseHandler() : _status(RES_IDLE), _pos(0) {}

void HttpResponseHandler::_makeStatusLine(StatusLine &statusLine, short code)
{
    std::string text;

    statusLine.version = std::make_pair(1, 1);
    statusLine.code = code;

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
    case 400:
        text = "Bad Request";
        break;
    case 404:
        text = "Not Found";
        break;
    case 500:
        text = "Internal Server Error";
        break;
    case 503:
        text = "Service Unavailable";
        break;
    case 505:
        text = "HTTP Version Not Supported";
        break;
    default:
        text = "Not Yet Setted\n";
    }

    statusLine.text = text;
}

void HttpResponseHandler::_setAllow()
{
    _httpResponse.headerFields.insert(std::make_pair("Allow", "GET, HEAD, POST, DELETE"));
}

void HttpResponseHandler::_setLastModified(const char *path)
{
    struct stat fileInfo;

    if (path[0] == '\0')
        return;
    if (stat(path, &fileInfo) == -1)
        return;
    std::time_t lastModifiedTime = fileInfo.st_mtime;
    if (lastModifiedTime == -1)
        return;
    std::tm *timeInfo = std::gmtime(&lastModifiedTime);
    if (timeInfo == NULL)
        return;

    char buf[100];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    _httpResponse.headerFields.insert(std::make_pair("Last-Modified", std::string(buf)));
}

void HttpResponseHandler::_setDate()
{
    std::time_t currentDate = std::time(NULL);
    if (currentDate == -1)
        return;
    std::tm *timeInfo = std::gmtime(&currentDate);
    if (timeInfo == NULL)
        return;

    char buf[100];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    _httpResponse.headerFields.insert(std::make_pair("Date", std::string(buf)));
}

// 수정 필요
void HttpResponseHandler::_setContentType()
{
    std::string type;

    if (1)
        type = "text/html";
    else if (1)
        type = "text/plain";
    else if (1)
        type = "image/jpeg";
    else if (1)
        type = "image/png";
    else
        type = "application/json";

    _httpResponse.headerFields.insert(std::make_pair("Content-Type", type));
}

void HttpResponseHandler::_setContentLength()
{
    // chunked로 구현하기?
    _httpResponse.headerFields.insert(std::make_pair("Content-Length", toString(_httpResponse.messageBody.length())));
}

void HttpResponseHandler::_setConnection(bool disConnected)
{
    if (disConnected)
        _httpResponse.headerFields.insert(std::make_pair("Connection", "close"));
    else
        _httpResponse.headerFields.insert(std::make_pair("Connection", "keep-alive"));
}

void HttpResponseHandler::_setLocation(std::string &location)
{
    _httpResponse.headerFields.insert(std::make_pair("Location", location));
}

void HttpResponseHandler::_makeHeaderFields(ConfigInfo &configInfo)
{
    _setAllow();
    _setConnection(1);
    _setContentLength();
    _setContentType();
    _setDate();
    _setLastModified(configInfo.getPath().c_str());
    _setLocation();
}

void HttpResponseHandler::_makeGETResponse(HttpRequest &httpRequest, ConfigInfo &configInfo, bool isGET)
{
    std::string path = "." + configInfo.getPath() + "index.html";
    int fileFd = open(path.c_str(), O_RDONLY);
    std::string messageBody;

    if (fileFd == -1) {
        _makeStatusLine(_httpResponse.statusLine, 404);
        fileFd = open(configInfo.getErrorPage().c_str(), O_RDONLY);
        // 404 Not Found 페이지가 없는 경우
        if (fileFd == -1)
            throw std::runtime_error("404 Not Found 페이지가 없습니다.");
    }
    else {
        _makeStatusLine(_httpResponse.statusLine, 200);
    }

    if (isGET) {
        char buffer[1024];
        memset(buffer, 0, 1024);
        while (read(fileFd, buffer, 1024) > 0) {
            messageBody.append(buffer);
            memset(buffer, 0, 1024);
        }
        _httpResponse.messageBody = messageBody;
    }

    // Header Field들을 세팅해준다.
    _makeHeaderFields(configInfo);

    close(fileFd);
}

void HttpResponseHandler::_makePOSTResponse(HttpRequest &httpRequest, ConfigInfo &configInfo)
{
    (void)httpRequest;
    (void)configInfo;
}

void HttpResponseHandler::_makeDELETEResponse(HttpRequest &httpRequest, ConfigInfo &configInfo)
{
    (void)httpRequest;
    (void)configInfo;
}

void HttpResponseHandler::_statusLineToString()
{
    const std::pair<short, short> version = _httpResponse.statusLine.version;
    std::string versionStr;
    std::string codeStr;

    versionStr = "HTTP/";
    versionStr.push_back(static_cast<char>(version.first + '0'));
    versionStr.push_back('.');
    versionStr.push_back(static_cast<char>(version.second + '0'));

    codeStr = toString(_httpResponse.statusLine.code);

    _response = versionStr + " " + codeStr + " " + _httpResponse.statusLine.text + CRLF;
}

void HttpResponseHandler::_headerFieldsToString()
{
    std::multimap<std::string, std::string>::iterator it = _httpResponse.headerFields.begin();
    for (; it != _httpResponse.headerFields.end(); it++) {
        _response += it->first + ": " + it->second + CRLF;
    }
    _response += CRLF;
}

void HttpResponseHandler::_httpResponseToString()
{
    _statusLineToString();
    _headerFieldsToString();
    _response += _httpResponse.messageBody;
    std::cout << "final result: " << _response << '\n';
}

void HttpResponseHandler::makeHttpErrorResponse(short code)
{
    _makeStatusLine(_httpResponse.statusLine, code);
    _makeHeaderFields();
    _httpResponse.messageBody = "";
    _httpResponseToString();
}

void HttpResponseHandler::makeHttpResponse(HttpRequest &httpRequest, ConfigInfo &configInfo)
{
    const unsigned short code = httpRequest.getCode();

    // http request에서 이미 에러가 발생한 경우
    if (code != 0) {
        makeHttpErrorResponse(code);
    }
    else {
        if (configInfo.getIsRedirect()) {
            // 빈문자열인지 확인하기
        }
        const short method = httpRequest.getRequestLine().getMethod();
        switch (method) {
        case GET:
            _makeGETResponse(httpRequest, configInfo, true);
            break;
        case HEAD:
            _makeGETResponse(httpRequest, configInfo, false);
            break;
        case POST:
            _makePOSTResponse(httpRequest, configInfo);
            break;
        case DELETE:
            _makeDELETEResponse(httpRequest, configInfo);
            break;
        default:
            // 여기로 올 수 있는 경우는 이미 HttpRequestHandler에서 걸러졌다.
        }
    }

    _httpResponseToString();
}

void HttpResponseHandler::sendHttpResponse(int fd, size_t size)
{
    size_t writeLen;

    writeLen = std::min(_response.size() - _pos, size);
    if (write(fd, _response.c_str() + _pos, writeLen) == FAILURE)
        throw std::runtime_error("sendHttpResponse에서 write 실패");
    _pos += writeLen;
    if (_pos == size) {
        _status = RES_IDLE;
        _pos = 0;
    }
}