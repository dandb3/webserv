#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include "HttpResponseModule.hpp"
#include "../../utils/utils.hpp"
#include "../parse/parse.hpp"

HttpResponseHandler::HttpResponseHandler() : _status(RES_IDLE), _pos(0) {}

bool HttpResponseHandler::isErrorCode(unsigned short code)
{
    return (code >= 400 && code < 600);
}

void HttpResponseHandler::_makeStatusLine()
{
    _httpResponse.statusLine.version = std::make_pair(1, 1);
    switch (_httpResponse.statusLine.code) {
    case 100:
        _httpResponse.statusLine.text = "Continue";
        break;
    case 101:
        _httpResponse.statusLine.text = "Switching Protocol";
        break;
    case 102:
        _httpResponse.statusLine.text = "Processing";
        break;
    case 200:
        _httpResponse.statusLine.text = "OK";
        break;
    case 201:
        _httpResponse.statusLine.text = "Created";
        break;
    case 202:
        _httpResponse.statusLine.text = "Accepted";
        break;
    case 203:
        _httpResponse.statusLine.text = "Non-Authoritative Information";
        break;
    case 204:
        _httpResponse.statusLine.text = "No Content";
        break;
    case 400:
        _httpResponse.statusLine.text = "Bad Request";
        break;
    case 403:
        _httpResponse.statusLine.text = "Forbidden";
        break;
    case 404:
        _httpResponse.statusLine.text = "Not Found";
        break;
    case 405:
        _httpResponse.statusLine.text = "Method Not Allowed";
        break;
    case 408:
        _httpResponse.statusLine.text = "Request Timeout";
        break;
    case 409:
        _httpResponse.statusLine.text = "Conflict";
        break;
    case 413:
        _httpResponse.statusLine.text = "Payload Too Large";
        break;
    case 414:
        _httpResponse.statusLine.text = "URI Too Long";
        break;
    case 500:
        _httpResponse.statusLine.text = "Internal Server Error";
        break;
    case 502:
        _httpResponse.statusLine.text = "Bad Gateway";
        break;
    case 503:
        _httpResponse.statusLine.text = "Service Unavailable";
        break;
    case 504:
        _httpResponse.statusLine.text = "Gateway Timeout";
        break;
    case 505:
        _httpResponse.statusLine.text = "HTTP Version Not Supported";
        break;
    default:
        _httpResponse.statusLine.text = "Not Set";
        break;
    }
}

void HttpResponseHandler::_setAllow(ConfigInfo& configInfo)
{
    const char* methods[4] = {"GET", "HEAD", "POST", "DELETE"};
    std::string value;

    for (int i = 0; i < 4; ++i) {
        if (configInfo.getAllowMethods(i))
            value += std::string(methods[i]) + ", ";
    }
    if (!value.empty())
        value.resize(value.size() - 2);
    _httpResponse.headerFields.insert(std::make_pair("Allow", value));
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

void HttpResponseHandler::_makeDirectoryListing(const std::string& path)
{
    DIR *dir = opendir(path.c_str());
    struct dirent *entry;

    if (dir == NULL)
        throw 500;

    _httpResponse.messageBody = "<!DOCTYPE html>\n<html>\n<head>\n<title>Directory Listing</title>\n</head>\n<body>\n";
    _httpResponse.messageBody += "<h1>Directory Listing: " + path + "</h1>\n<ul>\n";

    while ((entry = readdir(dir)) != NULL)
        if (entry->d_name[0] != '.')  // Skip hidden files/directories
            _httpResponse.messageBody += std::string("<li><a href=\"") + entry->d_name + "\">" + entry->d_name + "</a></li>\n";

    _httpResponse.messageBody += "</ul>\n</body>\n</html>\n";

    closedir(dir);
}

void HttpResponseHandler::_setDate()
{
    std::time_t currentDate = std::time(NULL);

    if (_httpResponse.headerFields.find("Date") != _httpResponse.headerFields.end())
        return;
    if (currentDate == -1)
        return;
    std::tm *timeInfo = std::gmtime(&currentDate);

    if (timeInfo == NULL)
        return;

    char buf[100];

    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    _httpResponse.headerFields.insert(std::make_pair("Date", std::string(buf)));
}

// 수정 필요, mimeTypes의 구조.
void HttpResponseHandler::_setContentType(Cycle* cycle, const std::string& path)
{
    ConfigInfo& configInfo = cycle->getConfigInfo();
    std::map<std::string, std::string>& mimeTypes = Config::getInstance().getMimeTypes();
    std::string extension;
    std::string contentType;
    size_t extensionPos;

    extension = path.substr(path.find_last_of('/') + 1); // path에 '/'가 존재한다고 가정.
    if ((extensionPos = extension.find_last_of('.')) == std::string::npos) { // 확장자가 없는 경우
        contentType = "application/octet-stream";
    }
    else { // 확장자가 있는 경우
        extension = extension.substr(extensionPos + 1);
        if (mimeTypes.find(extension) == mimeTypes.end())
            contentType = "application/octet-stream";
        else
            contentType = mimeTypes.at(extension);
    }
    _httpResponse.headerFields.insert(std::make_pair("Content-Type", contentType));
}

void HttpResponseHandler::_setContentType(const std::string& type)
{
    _httpResponse.headerFields.insert(std::make_pair("Content-Type", type));
}

void HttpResponseHandler::_setContentLength()
{
    _httpResponse.headerFields.insert(std::make_pair("Content-Length", toString(_httpResponse.messageBody.length())));
}

void HttpResponseHandler::_setContentLength(off_t size)
{
    _httpResponse.headerFields.insert(std::make_pair("Content-Length", toString(size)));
}

// 수정 필요
void HttpResponseHandler::_setConnection(Cycle* cycle)
{
    _httpResponse.headerFields.erase("Connection");
    if (cycle->getHttpRequestQueue().empty() && cycle->closed())
        _httpResponse.headerFields.insert(std::make_pair("Connection", "close"));
    else
        _httpResponse.headerFields.insert(std::make_pair("Connection", "keep-alive"));
}

void HttpResponseHandler::_makeHeaderFields(Cycle* cycle)
{
    _setConnection(cycle);
    _setDate();
}

void HttpResponseHandler::_makeGETResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    std::string path = cycle->getConfigInfo().getPath();
    std::string prevPath;
    struct stat buf;
    int fd;

    if (access(path.c_str(), F_OK) == FAILURE)
        throw 404;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    if (S_ISDIR(buf.st_mode)) {
        prevPath = path;
        path += cycle->getConfigInfo().getIndex();
        
        if (access(path.c_str(), F_OK) == FAILURE) {
            if (cycle->getConfigInfo().getAutoIndex() == false)
                throw 404;
            if (access(prevPath.c_str(), R_OK) == FAILURE)
                throw 403;
            _makeDirectoryListing(prevPath);
            _setContentType("text/html");
            _setContentLength();
            _httpResponse.statusLine.code = 200;
            makeHttpResponseFinal(cycle);
            return;
        }
        if (stat(path.c_str(), &buf) == FAILURE)
            throw 500;
        if (S_ISDIR(buf.st_mode)) {
            if (cycle->getConfigInfo().getAutoIndex() == false)
                throw 404;
            if (access(path.c_str(), R_OK) == FAILURE)
                throw 403;
            _makeDirectoryListing(path);
            _setContentType("text/html");
            _setContentLength();
            _httpResponse.statusLine.code = 200;
            makeHttpResponseFinal(cycle);
            return;
        }
        // index 파일이 존재하면서 디렉터리가 아닌 경우 if문을 빠져나온다.
    }
    if (access(path.c_str(), R_OK) == FAILURE)
        throw 403;
    if ((fd = open(path.c_str(), O_RDONLY)) == FAILURE)
        throw 500;
    fcntl(fd, F_SETFL, O_NONBLOCK);
    cycle->setReadFile(fd);
    _setContentType(cycle, path);
}

void HttpResponseHandler::_makeHEADResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    std::string path = cycle->getConfigInfo().getPath();
    std::string prevPath;
    struct stat buf;

    if (access(path.c_str(), F_OK) == FAILURE)
        throw 404;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    if (S_ISDIR(buf.st_mode)) {
        prevPath = path;
        path += cycle->getConfigInfo().getIndex();

        if (access(path.c_str(), F_OK) == FAILURE) {
            if (cycle->getConfigInfo().getAutoIndex() == false)
                throw 404;
            if (access(prevPath.c_str(), R_OK) == FAILURE)
                throw 403;
            _makeDirectoryListing(prevPath);
            _setContentType("text/html");
            _setContentLength();
            _httpResponse.statusLine.code = 200;
            _httpResponse.messageBody.clear();
            makeHttpResponseFinal(cycle);
            return;
        }
        if (stat(path.c_str(), &buf) == FAILURE)
            throw 500;
        if (S_ISDIR(buf.st_mode)) {
            if (cycle->getConfigInfo().getAutoIndex() == false)
                throw 404;
            if (access(path.c_str(), R_OK) == FAILURE)
                throw 403;
            _makeDirectoryListing(path);
            _setContentType("text/html");
            _setContentLength();
            _httpResponse.statusLine.code = 200;
            _httpResponse.messageBody.clear();
            makeHttpResponseFinal(cycle);
            return;
        }
        // index 파일이 존재하면서 디렉터리가 아닌 경우 if문을 빠져나온다.
    }
    if (access(path.c_str(), R_OK) == FAILURE)
        throw 403;
    _setContentType(cycle, path);
    _setContentLength(buf.st_size);
    _httpResponse.statusLine.code = 200;
    makeHttpResponseFinal(cycle);
}

void HttpResponseHandler::_makePOSTResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    std::map<std::string, std::string> files;
    const std::string contentType = httpRequest.getHeaderFields().find("Content-Type")->second;
    const std::string body = httpRequest.getMessageBody();
    const std::string fileName = httpRequest.getRequestLine().getUri();
    std::string fileContent;

    if (contentType == "") {
        if (body == "") { // 실제 body가 없는 경우
            _httpResponse.statusLine.code = 200;
            makeHttpResponseFinal(cycle);
            return;
        }
    }

    // 만약 같은 경로의 POST 요청이 존재한다면 throw 405;
    if (checkString(contentType, "multipart/form-data", 0)) {
        parseMultiForm(contentType, body, files);
    }
    else {
        if (checkString(contentType, "application/x-www-form-urlencoded", 0))
            fileContent = parseUrlencode(body);
        else if (checkString(contentType, "text/plain", 0)) {
            fileContent = body;
            parseTextPlain(fileContent);
        }
        else
            fileContent = body;
        files.insert(std::make_pair(fileName, fileContent));
    }

    std::map<int, WriteFile>& writeFiles = cycle->getWriteFiles();
    std::map<std::string, std::string>::iterator it;
    std::map<int, WriteFile>::iterator fileIt;
    int fd;

    for (it = files.begin(); it != files.end(); ++it) {
        if (access(it->first.c_str(), F_OK) == SUCCESS)
            throw 409;
        if (access(dirPath(it->first).c_str(), W_OK | X_OK) == FAILURE)
            throw 409;
    }
    for (it = files.begin(); it != files.end(); ++it) {
        fd = open(it->first.c_str(), O_WRONLY | O_CREAT, 0644);
        if (fd == FAILURE) {
            for (fileIt = writeFiles.begin(); fileIt != writeFiles.end(); ++fileIt) {
                close(fileIt->first);
                std::remove(fileIt->second.getPath().c_str());
            }
            writeFiles.clear();
            throw 500;
        }
        writeFiles.insert(std::make_pair(fd, WriteFile(it->first, it->second)));
    }
}

void HttpResponseHandler::_makeDELETEResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    std::string path = cycle->getConfigInfo().getPath();
    struct stat buf;

    if (access(path.c_str(), F_OK) == FAILURE)
        throw 404;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    if (S_ISDIR(buf.st_mode))
        throw 403;
    if (access(dirPath(path).c_str(), W_OK | X_OK) == FAILURE)
        throw 403;
    if (std::remove(path.c_str()) == FAILURE)
        throw 500;
    _httpResponse.statusLine.code = 204;
    makeHttpResponseFinal(cycle);
}

void HttpResponseHandler::makeErrorHttpResponse(Cycle* cycle)
{
    const std::string& errorPage = cycle->getConfigInfo().getErrorPage(toString(_httpResponse.statusLine.code));
    int fd;

    if (access(errorPage.c_str(), R_OK) == FAILURE \
        || (fd = open(errorPage.c_str(), O_RDONLY)) == FAILURE) {
        if (errorPage == ConfigInfo::getDefaultPage(_httpResponse.statusLine.code)) {
            _httpResponse.statusLine.code = 500;
            makeHttpResponseFinal(cycle);
        }
        else {
            cycle->getConfigInfo().setDefaultErrorPage(_httpResponse.statusLine.code);
            makeErrorHttpResponse(cycle);
        }
    }
    else {
        fcntl(fd, F_SETFL, O_NONBLOCK);
        cycle->setReadFile(fd);
        _setContentType(cycle, errorPage);
    }
}

/**
 * code를 기반으로 status-line 생성
 * message-body를 기반으로 Content-Length 설정 및 기본 header-fields 설정 (date, 등등)
 * message-body는 그냥 그대로 유지한다.
*/
void HttpResponseHandler::makeHttpResponseFinal(Cycle* cycle)
{
    _makeStatusLine();
    _makeHeaderFields(cycle);
    _statusLineToString();
    _headerFieldsToString();
    _httpResponseToString();
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

void HttpResponseHandler::makeHttpResponse(Cycle* cycle, HttpRequest &httpRequest)
{
    ConfigInfo& configInfo = cycle->getConfigInfo();
    const std::string& path = configInfo.getPath();
    const short method = httpRequest.getRequestLine().getMethod();

    _httpResponse.statusLine.code = httpRequest.getCode();
    if (isErrorCode(_httpResponse.statusLine.code)) {
        makeErrorHttpResponse(cycle);
        return;
    }
    if (configInfo.getIsRedirect()) {
        _httpResponse.statusLine.code = stringToType<unsigned short>(configInfo.getRedirect().first);
        _httpResponse.headerFields.insert(std::make_pair("Location", configInfo.getRedirect().second));
        makeErrorHttpResponse(cycle);
        return;
    }
    try {
        switch (method) {
        case GET:
            if (!configInfo.getAllowMethods(0)) {
                _setAllow(configInfo);
                throw 405;
            }
            _makeGETResponse(cycle, httpRequest);
            break;
        case HEAD:
            if (!configInfo.getAllowMethods(1)) {
                _setAllow(configInfo);
                throw 405;
            }
            _makeHEADResponse(cycle, httpRequest);
            break;
        case POST:
            if (!configInfo.getAllowMethods(2)) {
                _setAllow(configInfo);
                throw 405;
            }
            _makePOSTResponse(cycle, httpRequest);
            break;
        case DELETE:
            if (!configInfo.getAllowMethods(3)) {
                _setAllow(configInfo);
                throw 405;
            }
            _makeDELETEResponse(cycle, httpRequest);
            break;
        }
    }
    catch (unsigned short code) {
        _httpResponse.statusLine.code = code;
        makeErrorHttpResponse(cycle);
    }
}

void HttpResponseHandler::makeHttpResponse(Cycle* cycle, CgiResponse &cgiResponse)
{
    const std::vector<pair_t>& cgiHeaderFields = cgiResponse.getHeaderFields();
    std::vector<pair_t>::const_iterator it = cgiHeaderFields.begin();

    _httpResponse.statusLine.code = cgiResponse.getStatusCode();
    if (isErrorCode(_httpResponse.statusLine.code)) {
        makeErrorHttpResponse(cycle);
        return;
    }

    _httpResponse.messageBody = cgiResponse.getMessageBody();
    for (; it != cgiHeaderFields.end(); ++it)
        if (!isCaseInsensitiveSame(it->first, "Status") && !isCaseInsensitiveSame(it->first, "Content-Length"))
            _httpResponse.headerFields.insert(*it);

    // 아래 함수가 기본적으로 만들어 주는 header-field가 있을텐데, 이 때 어떤 field가 만들어지는지 확실히 해야 한다.
    makeHttpResponseFinal(cycle);
}

void HttpResponseHandler::sendHttpResponse(int fd, size_t size)
{
    size_t writeLen;

    writeLen = std::min(_response.size() - _pos, size);
    if (write(fd, _response.c_str() + _pos, writeLen) == FAILURE)
        throw std::runtime_error("sendHttpResponse에서 write 실패");
    _pos += writeLen;
    if (_pos == size) {
        _status = RES_FINISH;
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

void HttpResponseHandler::reset()
{
    _response.clear();
    _pos = 0;
    _status = RES_IDLE;
    _httpResponse.statusLine.code = 0;
    _httpResponse.statusLine.text.clear();
    _httpResponse.headerFields.clear();
    _httpResponse.messageBody.clear();
}
