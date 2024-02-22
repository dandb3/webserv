#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include "HttpResponseModule.hpp"
#include "../parse/parse.hpp"
#include "post.hpp"

#include <iostream> // test ??

HttpResponseHandler::HttpResponseHandler() : _pos(0), _status(RES_IDLE) {}

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
    case 501:
        _httpResponse.statusLine.text = "Not Implemented";
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

    std::stringstream bodyStream;
    bodyStream << "<!DOCTYPE html>\n<html>\n<head>\n<title>Directory Listing</title>\n</head>\n<body>\n";
    bodyStream << "<h1>Directory Listing: " << path << "</h1>\n<ul>\n";

    while ((entry = readdir(dir)) != NULL)
        if (entry->d_name[0] != '.')  // Skip hidden files/directories
            bodyStream << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>\n";

    bodyStream << "</ul>\n</body>\n</html>\n";

    _httpResponse.messageBody = bodyStream.str();

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
void HttpResponseHandler::_setContentType(bool isPath, const std::string& str)
{
    if (!isPath) {
        _httpResponse.headerFields.insert(std::make_pair("Content-Type", str));
        return;
    }
    std::map<std::string, std::string>& mimeTypes = Config::getInstance().getMimeTypes();
    std::string extension;
    std::string contentType;
    size_t extensionPos;

    extension = str.substr(str.find_last_of('/') + 1); // path에 '/'가 존재한다고 가정.
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

void HttpResponseHandler::_setContentLength()
{
    _httpResponse.headerFields.insert(std::make_pair("Content-Length", toString(_httpResponse.messageBody.length())));
}

void HttpResponseHandler::_setContentLength(off_t size)
{
    _httpResponse.headerFields.insert(std::make_pair("Content-Length", toString(size)));
}

// 수정 필요
void HttpResponseHandler::_setConnection(ICycle* cycle)
{
    _httpResponse.headerFields.erase("Connection");
    if (cycle->getHttpRequestQueue().empty() && cycle->closed())
        _httpResponse.headerFields.insert(std::make_pair("Connection", "close"));
    else
        _httpResponse.headerFields.insert(std::make_pair("Connection", "keep-alive"));
}

void HttpResponseHandler::_makeHeaderFields(ICycle* cycle)
{
    _setAllow(cycle->getConfigInfo());
    _setConnection(cycle);
    _setDate();
}

void HttpResponseHandler::_makeGETResponse(ICycle* cycle)
{
    std::string path = cycle->getConfigInfo().getPath();
    std::string prevPath;
    struct stat buf;
    int fd;

    std::cout << "access path: " << path << std::endl; // test
    if (access(path.c_str(), F_OK) == FAILURE)
        throw 404;
    if (stat(path.c_str(), &buf) == FAILURE)
        throw 500;
    
    if (S_ISDIR(buf.st_mode)) {
        prevPath = path;
        if (path.back() != '/')
            path.push_back('/');
        path += cycle->getConfigInfo().getIndex();
        
        if (access(path.c_str(), F_OK) == FAILURE) {
            if (cycle->getConfigInfo().getAutoIndex() == false)
                throw 404;
            if (access(prevPath.c_str(), R_OK) == FAILURE)
                throw 403;
            _makeDirectoryListing(prevPath);
            _setContentType(false, "text/html");
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
            _setContentType(false, "text/html");
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
    _setContentType(true, path);
    _setLastModified(path.c_str());
}

void HttpResponseHandler::_makeHEADResponse(ICycle* cycle)
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
            _setContentType(false, "text/html");
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
            _setContentType(false, "text/html");
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
    _setContentType(true, path);
    _setContentLength(buf.st_size);
    _setLastModified(path.c_str());
    _httpResponse.statusLine.code = 200;
    makeHttpResponseFinal(cycle);
}

void HttpResponseHandler::_makePOSTResponse(ICycle* cycle, HttpRequest &httpRequest)
{
    std::map<std::string, std::string> newFiles, files;
    const std::string contentType = httpRequest.getHeaderFields().find("Content-Type")->second;
    const std::string body = httpRequest.getMessageBody();
    const std::string fileName = cycle->getConfigInfo().getPath();
    std::string fileContent;

    if (contentType == "") {
        if (body == "") { // 실제 body가 없는 경우
            _httpResponse.statusLine.code = 204;
            makeHttpResponseFinal(cycle);
            return;
        }
    }

    // 만약 같은 경로의 POST 요청이 존재한다면 throw 409;
    if (checkString(contentType, "multipart/form-data", 0)) {
        parseMultiForm(contentType, body, newFiles);
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
        newFiles.insert(std::make_pair(fileName, fileContent));
    }

    std::map<int, WriteFile>& writeFiles = cycle->getWriteFiles();
    std::map<std::string, std::string>::iterator it;
    std::map<int, WriteFile>::iterator fileIt;
    std::string path;
    struct stat buf;
    int fd;

    for (it = newFiles.begin(); it != newFiles.end();) {
        // if (access(it->first.c_str(), F_OK) == SUCCESS)
        //     throw 409;
        path = it->first;
        if (access(it->first.c_str(), F_OK) == SUCCESS) {
            if (stat(it->first.c_str(), &buf) == FAILURE)
                throw 500;
            if (S_ISDIR(buf.st_mode)) {
                if (it->first.back() == '/')
                    path += cycle->getConfigInfo().getIndex();
                else
                    path += "/" + cycle->getConfigInfo().getIndex();
            }
        }
        files.insert(std::make_pair(path, it->second));
        if (access(dirPath(path).c_str(), W_OK | X_OK) == FAILURE)
        // if (access("wow", W_OK | X_OK) == FAILURE)
            throw 403;
        ++it;
    }
    for (it = files.begin(); it != files.end(); ++it) {
        fd = open(it->first.c_str(), O_WRONLY | O_CREAT, 0644);
        // fd = open("wow/eng.txt", O_WRONLY | O_CREAT, 0644);
        if (fd == FAILURE) {
            for (fileIt = writeFiles.begin(); fileIt != writeFiles.end(); ++fileIt) {
                close(fileIt->first);
                std::remove(fileIt->second.getPath().c_str());
            }
            writeFiles.clear();
            throw 500;
        }
        fcntl(fd, F_SETFL, O_NONBLOCK);
        writeFiles.insert(std::make_pair(fd, WriteFile(it->first, it->second)));
    }
}

void HttpResponseHandler::_makeDELETEResponse(ICycle* cycle)
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

void HttpResponseHandler::_makeRedirectHttpResponse(ICycle* cycle)
{
    ConfigInfo& configInfo = cycle->getConfigInfo();

    _httpResponse.statusLine.code = stringToType<unsigned short>(configInfo.getRedirect().first);
    _httpResponse.headerFields.insert(std::make_pair("Location", configInfo.getRedirect().second));
    _httpResponse.messageBody.append(
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta http-equiv=\"refresh\" content=\"0; URL="
    );
    _httpResponse.messageBody.append(configInfo.getRedirect().second.c_str());
    _httpResponse.messageBody.append(
        "\">\n"
        "    <title>301 Moved Permanently</title>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>301 Moved Permanently</h1>\n"
        "    <p>This resource has been moved to <a href=\""
    );
    _httpResponse.messageBody.append(configInfo.getRedirect().second.c_str());
    _httpResponse.messageBody.append("\">");
    _httpResponse.messageBody.append(configInfo.getRedirect().second.c_str());
    _httpResponse.messageBody.append(
        "</a>.</p>\n"
        "</body>\n"
        "</html>\n"
    );
    makeHttpResponseFinal(cycle);
}

void HttpResponseHandler::makeErrorHttpResponse(ICycle* cycle)
{
    const std::string& errorPage = cycle->getConfigInfo().getErrorPage(toString(_httpResponse.statusLine.code));
    int fd;

    if (access(errorPage.c_str(), R_OK) == FAILURE \
        || (fd = open(errorPage.c_str(), O_RDONLY)) == FAILURE) {
        if (errorPage == ConfigInfo::getDefaultPage(_httpResponse.statusLine.code))
            makeHttpResponseFinal(cycle);
        else {
            std::cout << "에러 페이지를 읽어올 수 없습니다.\n"; // test
            cycle->getConfigInfo().setDefaultErrorPage(_httpResponse.statusLine.code);
            makeErrorHttpResponse(cycle);
        }
    }
    else {
        std::cout << "에러 페이지를 읽어옵니다.\n"; // test
        fcntl(fd, F_SETFL, O_NONBLOCK);
        cycle->setReadFile(fd);
        _setContentType(true, errorPage);
    }
    std::cout << "make error response finish\n"; // test
}

/**
 * code를 기반으로 status-line 생성
 * message-body를 기반으로 Content-Length 설정 및 기본 header-fields 설정 (date, 등등)
 * message-body는 그냥 그대로 유지한다.
*/
void HttpResponseHandler::makeHttpResponseFinal(ICycle* cycle)
{
    std::stringstream responseStream;

    _makeStatusLine();
    _makeHeaderFields(cycle);
    _httpResponseToString();
}

void HttpResponseHandler::_statusLineToString(std::stringstream &responseStream)
{
    const std::pair<short, short> version = _httpResponse.statusLine.version;

    responseStream << "HTTP/" << version.first << '.' << version.second << ' ';
    responseStream << _httpResponse.statusLine.code << ' ';
    responseStream << _httpResponse.statusLine.text << CRLF;
}

void HttpResponseHandler::_headerFieldsToString(std::stringstream &responseStream)
{
    std::multimap<std::string, std::string>::iterator it = _httpResponse.headerFields.begin();

    responseStream << _response;
    for (; it != _httpResponse.headerFields.end(); ++it)
        responseStream << it->first << ": " << it->second << CRLF;
    responseStream << CRLF;
}

void HttpResponseHandler::_httpResponseToString()
{
    std::stringstream responseStream;

    _statusLineToString(responseStream);
    _headerFieldsToString(responseStream);
    responseStream << _httpResponse.messageBody;
    
    _response = responseStream.str();
}

void HttpResponseHandler::makeHttpResponse(ICycle* cycle, HttpRequest &httpRequest)
{
    ConfigInfo& configInfo = cycle->getConfigInfo();
    const short method = httpRequest.getRequestLine().getMethod();

    _httpResponse.statusLine.code = httpRequest.getCode();
    if (isErrorCode(_httpResponse.statusLine.code)) {
        makeErrorHttpResponse(cycle);
        return;
    }
    if (configInfo.getIsRedirect()) {
        _makeRedirectHttpResponse(cycle);
        return;
    }
    try {
        std::cout << "switch method : " << method << "\n"; // test
        switch (method) {
        case GET:
            if (!configInfo.getAllowMethods(0)) {
                // _setAllow(configInfo);
                throw 405;
            }
            _makeGETResponse(cycle);
            break;
        case HEAD:
            if (!configInfo.getAllowMethods(1)) {
                // _setAllow(configInfo);
                throw 405;
            }
            _makeHEADResponse(cycle);
            break;
        case POST:
            if (!configInfo.getAllowMethods(2)) {
                // _setAllow(configInfo);
                throw 405;
            }
            _makePOSTResponse(cycle, httpRequest);
            break;
        case DELETE:
            if (!configInfo.getAllowMethods(3)) {
                // _setAllow(configInfo);
                throw 405;
            }
            _makeDELETEResponse(cycle);
            break;
        }
    }
    catch (int code) {
        std::cout << "http response handler make http response catch: " << code << "\n"; // test
        _httpResponse.statusLine.code = static_cast<short>(code);
        makeErrorHttpResponse(cycle);
    }
    std::cout << "makeHttpResponse finish\n"; // test
}

void HttpResponseHandler::makeHttpResponse(ICycle* cycle, CgiResponse &cgiResponse)
{
    const std::vector<pair_t>& cgiHeaderFields = cgiResponse.getHeaderFields();
    std::vector<pair_t>::const_iterator it = cgiHeaderFields.begin();

    _httpResponse.statusLine.code = cgiResponse.getStatusCode();
    if (isErrorCode(_httpResponse.statusLine.code)) {
        makeErrorHttpResponse(cycle);
        return;
    }

    _httpResponse.messageBody = cgiResponse.getMessageBody();
    _setContentLength(_httpResponse.messageBody.length());
    for (; it != cgiHeaderFields.end(); ++it)
        if (!isCaseInsensitiveSame(it->first, "Status") && !isCaseInsensitiveSame(it->first, "Content-Length"))
            _httpResponse.headerFields.insert(*it);

    // 아래 함수가 기본적으로 만들어 주는 header-field가 있을텐데, 이 때 어떤 field가 만들어지는지 확실히 해야 한다.
    makeHttpResponseFinal(cycle);
}

void HttpResponseHandler::sendHttpResponse(int fd, size_t size)
{
    size_t writeLen;

    // std::cout << "_response: " << _response << "\n"; // test
    writeLen = std::min(_response.size() - _pos, size);
    if ((writeLen = write(fd, _response.c_str() + _pos, writeLen)) == FAILURE)
        throw std::runtime_error("sendHttpResponse에서 write 실패");
    _pos += writeLen;
    if (_pos == _response.size()) {
        _status = RES_FINISH;
        _pos = 0;
    }
    // std::cout << "sendHttpResponse finish\n"; // test
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
