#include "HttpResponseModule.hpp"
#include "../parse/parse.hpp"

HttpResponseHandler::HttpResponseHandler() : _status(RES_IDLE), _pos(0) {}

bool HttpResponseHandler::_isErrorCode(unsigned short code)
{
    return (code >= 400 && code < 600);
}

void HttpResponseHandler::_makeStatusLine(StatusLine &statusLine, short code)
{
    std::string text;

    statusLine.setVersion(std::make_pair(1, 1));
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

void HttpResponseHandler::_setLastModified(std::multimap<std::string, std::string> &headerFields, const char *path)
{
    struct stat fileInfo;

    if (path == std::string(""))
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
    headerFields.insert(std::make_pair("Last-Modified", std::string(buf)));
}

void HttpResponseHandler::_setDate(std::multimap<std::string, std::string> &headerFields)
{
    std::time_t currentDate = std::time(NULL);
    if (currentDate == -1)
        return;
    std::tm *timeInfo = std::gmtime(&currentDate);
    if (timeInfo == NULL)
        return;

    char buf[100];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    headerFields.insert(std::make_pair("Date", std::string(buf)));
}

// 수정 필요
void HttpResponseHandler::_setContentType(std::multimap<std::string, std::string> &headerFields)
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

    headerFields.insert(std::make_pair("Content-Type", type));
}

void HttpResponseHandler::_setContentLength(std::multimap<std::string, std::string> &headerFields)
{
    // chunked로 구현하기?
    headerFields.insert(std::make_pair("Content-Length", toString(_httpResponse.getMessageBody().length())));
}

// 수정 필요
void HttpResponseHandler::_setConnection(std::multimap<std::string, std::string> &headerFields)
{
    if (1)
        headerFields.insert(std::make_pair("Connection", "keep-alive"));
    else
        headerFields.insert(std::make_pair("Connection", "close"));
}

void HttpResponseHandler::_makeHeaderFields(std::multimap<std::string, std::string> &headerFields, ConfigInfo &configInfo)
{
    _setConnection(headerFields);
    _setContentLength(headerFields);
    _setContentType(headerFields);
    _setDate(headerFields);
    _setLastModified(headerFields, configInfo.getPath().c_str());
}

void HttpResponseHandler::_makeGETResponse(HttpRequest &httpRequest, ConfigInfo &configInfo, bool isGET)
{
    std::string path = "." + configInfo.getPath() + "index.html";
    int fileFd = open(path.c_str(), O_RDONLY);
    StatusLine statusLine;
    std::multimap<std::string, std::string> headerFields;
    std::string messageBody;

    if (fileFd == -1) {
        _makeStatusLine(statusLine, 404);
        fileFd = open(configInfo.getErrorPage().c_str(), O_RDONLY);
        // 404 Not Found 페이지가 없는 경우
        if (fileFd == -1)
            throw std::runtime_error("404 Not Found 페이지가 없습니다.");
    }
    else {
        _makeStatusLine(statusLine, 200);
    }
    _httpResponse.setStatusLine(statusLine);

    if (isGET) {
        char buffer[1024];
        memset(buffer, 0, 1024);
        while (read(fileFd, buffer, 1024) > 0) {
            messageBody.append(buffer);
            memset(buffer, 0, 1024);
        }
        messageBody.push_back('\0');
        _httpResponse.setMessageBody(messageBody);
    }

    // Header Field들을 세팅해준다.
    _makeHeaderFields(headerFields, configInfo);
    _httpResponse.setHeaderFields(headerFields);

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

void HttpResponseHandler::_makeHttpErrorResponse(Cycle* cycle)
{
    const std::string& errorPage = cycle->getConfigInfo().getErrorPage();
    int fd;

    if (access(errorPage.c_str(), R_OK) == FAILURE \
        || (fd = open(errorPage.c_str(), O_RDONLY)) == FAILURE) {
        if (errorPage == ConfigInfo::getDefaultErrorPage()) {
            _httpResponse.statusLine.code = 500;
            _makeHttpResponseFinal();
        }
        else {
            cycle->getConfigInfo().setDefaultErrorPage();
            _makeHttpErrorResponse(cycle);
        }
    }
    else {
        fcntl(fd, F_SETFL, O_NONBLOCK);
        _addContentType(cycle->getConfigInfo());
        cycle->getOpenFiles().insert(fd);
    }
}

void HttpResponseHandler::_makeHttpResponseFinal()
{
    /**
     * code를 기반으로 status-line 생성
     * message-body를 기반으로 Content-Length 설정 및 기본 header-fields 설정 (date, 등등)
     * message-body는 그냥 그대로 유지한다.
    */
}

void HttpResponseHandler::_statusLineToString()
{
    const std::pair<short, short> version = _httpResponse.getStatusLine().getVersion();
    const short code = _httpResponse.getStatusLine().getCode();
    std::string versionStr;
    std::string codeStr;

    versionStr = "HTTP/";
    versionStr.push_back(static_cast<char>(version.first + '0'));
    versionStr.push_back('.');
    versionStr.push_back(static_cast<char>(version.second + '0'));

    codeStr = toString(code);

    _response = versionStr + " " + codeStr + " " + _httpResponse.getStatusLine().getText() + CRLF;
}

void HttpResponseHandler::_headerFieldsToString()
{
    std::multimap<std::string, std::string> headerFields = _httpResponse.getHeaderFields();
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
    std::cout << "final result: " << _response << '\n';
}

void HttpResponseHandler::makeHttpResponse(HttpRequest &httpRequest, ConfigInfo &configInfo)
{
    const short method = httpRequest.getRequestLine().getMethod();

    // if else -> switch?
    // httpRequest.code 확인해서 response 만들기
    if (method == GET) {
        _makeGETResponse(httpRequest, configInfo, true);
    }
    else if (method == HEAD) {
        _makeGETResponse(httpRequest, configInfo, false);
    }
    else if (method == POST) {
        _makePOSTResponse(httpRequest, configInfo);
    }
    else if (method == DELETE) {
        _makeDELETEResponse(httpRequest, configInfo);
    }
    else {
        return;
        // 400 Bad Request
    }

    _httpResponseToString();
}

void HttpResponseHandler::makeHttpResponse(Cycle* cycle, const CgiResponse &cgiResponse)
{
    const std::vector<pair_t>& cgiHeaderFields = cgiResponse.getHeaderFields();
    std::vector<pair_t>::const_iterator it = cgiHeaderFields.begin();

    _httpResponse.statusLine.code = cgiResponse.getStatusCode();
    if (_isErrorCode(_httpResponse.statusLine.code)) {
        _makeHttpErrorResponse(cycle);
        return;
    }

    _httpResponse.messageBody = cgiResponse.getMessageBody();
    for (; it != cgiHeaderFields.end(); ++it)
        if (!isCaseInsensitiveSame(it->first, "Status") && !isCaseInsensitiveSame(it->first, "Content-Length"))
            _httpResponse.headerFields.insert(*it);

    _makeHttpResponseFinal();
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

void HttpResponseHandler::setStatus(char status)
{
    _status = status;
}

char HttpResponseHandler::getStatus() const
{
    return _status;
}
