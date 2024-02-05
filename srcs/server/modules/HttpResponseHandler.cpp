#include <sys/stat.h>
#include "HttpResponseModule.hpp"
#include "../parse/parse.hpp"

HttpResponseHandler::HttpResponseHandler() : _status(RES_IDLE), _pos(0) {}

bool HttpResponseHandler::isErrorCode(unsigned short code)
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
void HttpResponseHandler::_setContentType(const std::string& path, ConfigInfo& configInfo)
{

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

void HttpResponseHandler::_makeGETResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    std::string path = cycle->getConfigInfo().getPath();
    struct stat buf;
    int fd;

    if (access(path.c_str(), F_OK) == FAILURE)
        throw 404;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    if (S_ISDIR(buf.st_mode)) {
        path = path + cycle->getConfigInfo().getIndex();
        
        if (access(path.c_str(), F_OK) == FAILURE) {
            if (configInfo.getAutoIndex() == false)
                throw 404;
            /**
             * directory listing 생성
             * code = 200;
             * Content-Type 설정
            */
            makeHttpResponseFinal();
            return;
        }
    }
    if (access(path.c_str(), R_OK) == FAILURE)
        throw 403;
    if ((fd = open(path.c_str(), O_RDONLY)) == FAILURE)
        throw 500;
    fcntl(fd, F_SETFL, O_NONBLOCK);
    cycle->setReadFile(fd);
    _addContentType(cycle->getConfigInfo());
}

void HttpResponseHandler::_makeHEADResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    std::string path = cycle->getConfigInfo().getPath();
    struct stat buf;

    if (access(path.c_str(), F_OK) == FAILURE)
        throw 404;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    if (S_ISDIR(buf.st_mode)) {
        path = path + cycle->getConfigInfo().getIndex();
        
        if (access(path.c_str(), F_OK) == FAILURE) {
            if (configInfo.getAutoIndex() == false)
                throw 404;
            /**
             * directory listing 생성
             * code = 200;
             * Content-Type 설정
            */
            makeHttpResponseFinal();
            return;
        }
    }
    if (access(path.c_str(), R_OK) == FAILURE)
        throw 403;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    _httpResponse.statusLine.code = 200;
    _httpResponse.headerFields.insert(std::make_pair("Content-Length", toString(buf.st_size)));
    _addContentType(path, cycle->getConfigInfo());
    makeHttpResponseFinal();
}

void HttpResponseHandler::_makePOSTResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    (void)httpRequest;
    (void)configInfo;
}

void HttpResponseHandler::_makeDELETEResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    std::string path = cycle->getConfigInfo().getPath();
    std::string dirPath = path.substr(0, path.find_last_of('/') + 1);
    struct stat buf;

    if (access(path.c_str(), F_OK) == FAILURE)
        throw 404;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    if (S_ISDIR(buf.st_mode))
        throw 403;
    if (access(dirPath.c_str(), W_OK | X_OK) == FAILURE)
        throw 403;
    if (std::remove(path.c_str()) == FAILURE)
        throw 500;
    _httpResponse.statusLine.code = 204;
    makeHttpResponseFinal();
}

void HttpResponseHandler::makeHttpErrorResponse(Cycle* cycle)
{
    const std::string& errorPage = cycle->getConfigInfo().getErrorPage();
    int fd;

    if (access(errorPage.c_str(), R_OK) == FAILURE \
        || (fd = open(errorPage.c_str(), O_RDONLY)) == FAILURE) {
        if (errorPage == ConfigInfo::getDefaultErrorPage()) {
            _httpResponse.statusLine.code = 500;
            makeHttpResponseFinal();
        }
        else {
            cycle->getConfigInfo().setDefaultErrorPage();
            makeHttpErrorResponse(cycle);
        }
    }
    else {
        fcntl(fd, F_SETFL, O_NONBLOCK);
        cycle->setReadFile(fd);
        _addContentType(cycle->getConfigInfo());
    }
}

void HttpResponseHandler::makeHttpResponseFinal()
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

void HttpResponseHandler::makeHttpResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    const std::string& path = httpRequest.getRequestLine().getUri();
    const short method = httpRequest.getRequestLine().getMethod();

    _httpResponse.statusLine.code = httpRequest.getCode();
    if (isErrorCode(_httpResponse.statusLine.code)) {
        makeHttpErrorResponse(cycle);
        return;
    }
    /**
     * redirect 확인
     * 맞으면?
     * _httpResponse.statusLine.code = 301;
     * 301 body 읽어와야 함.
     * Location 헤더 추가.
    */
    try {
        switch (method) {
        case GET:
            _makeGETResponse(httpRequest, cycle->getConfigInfo());
            break;
        case HEAD:
            _makeHEADResponse(httpRequest, cycle->getConfigInfo());
            break;
        case POST:
            _makePOSTResponse(httpRequest, cycle->getConfigInfo());
            break;
        case DELETE:
            _makeDELETEResponse(httpRequest, cycle->getConfigInfo());
            break;
        default:
            throw 405;
            break;
        }
    }
    catch (unsigned short code) {
        _httpResponse.statusLine.code = code;
        makeHttpErrorResponse(cycle);
    }
}

void HttpResponseHandler::makeHttpResponse(Cycle* cycle, const CgiResponse &cgiResponse)
{
    const std::vector<pair_t>& cgiHeaderFields = cgiResponse.getHeaderFields();
    std::vector<pair_t>::const_iterator it = cgiHeaderFields.begin();

    _httpResponse.statusLine.code = cgiResponse.getStatusCode();
    if (isErrorCode(_httpResponse.statusLine.code)) {
        makeHttpErrorResponse(cycle);
        return;
    }

    _httpResponse.messageBody = cgiResponse.getMessageBody();
    for (; it != cgiHeaderFields.end(); ++it)
        if (!isCaseInsensitiveSame(it->first, "Status") && !isCaseInsensitiveSame(it->first, "Content-Length"))
            _httpResponse.headerFields.insert(*it);

    makeHttpResponseFinal();
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

HttpResponse& HttpResponseHandler::getHttpResponse()
{
    return _httpResponse;
}
