#include <sstream>
#include "ConfigInfo.hpp"

#include <iostream> // for test ??

const std::pair<const std::string, std::string> defaultPages[] = {
    std::make_pair("400", "defaultPage/400_BadRequest.html"),
    std::make_pair("401", "defaultPage/401_Unauthorized.html"),
    std::make_pair("403", "defaultPage/403_Forbidden.html"),
    std::make_pair("404", "defaultPage/404_NotFound.html"),
    std::make_pair("405", "defaultPage/405_MethodNotAllowed.html"),
    std::make_pair("408", "defaultPage/408_RequestTimeout.html"),
    std::make_pair("409", "defaultPage/409_Conflict.html"),
    std::make_pair("413", "defaultPage/413_PayloadTooLarge.html"),
    std::make_pair("414", "defaultPage/414_URITooLong.html"),
    std::make_pair("500", "defaultPage/500_InternalServerError.html"),
    std::make_pair("502", "defaultPage/502_BadGateway.html"),
    std::make_pair("503", "defaultPage/503_ServiceUnavailable.html"),
    std::make_pair("504", "defaultPage/504_GatewayTimeout.html"),
    std::make_pair("505", "defaultPage/505_HTTPVersionNotSupported.html"),
    std::make_pair("default", "defaultPage/defaultError.html")
};

const std::string ConfigInfo::DEFAULT_INDEX = "index.html";
const std::string ConfigInfo::DEFAULT_ROOT = "var/www/html/";
const std::map<std::string, std::string> ConfigInfo::DEFAULT_PAGE(defaultPages, defaultPages + sizeof(defaultPages) / sizeof(defaultPages[0]));

const std::string& ConfigInfo::getDefaultPage(unsigned short code)
{
    if (ConfigInfo::DEFAULT_PAGE.find(toString(code)) == ConfigInfo::DEFAULT_PAGE.end())
        return ConfigInfo::DEFAULT_PAGE.at("default");
    return ConfigInfo::DEFAULT_PAGE.at(toString(code));
}

/// @brief ip, port를 보고 matchedServer,
///        uri를 보고 matchedLocation을 찾아서 ConfigInfo 생성
ConfigInfo::ConfigInfo()
{
    // ip, port을 보고 match되는 config를 찾아서 생성자 호출
    _root = DEFAULT_ROOT;
    _cgiPath = "";
    _path = "";
    for (size_t i = 0; i < 4; i++)
        _allowMethods[i] = true;
    _index = DEFAULT_INDEX;
    _errorPage.clear();
    _autoIndex = false;
    _isRedirect = false;
    _requestTimeout = DEFAULT_TIMEOUT;
    _keepaliveTimeout = DEFAULT_TIMEOUT;
    _redirect = std::make_pair("", "");
    _info.clear();
}

ConfigInfo::ConfigInfo(in_addr_t ip, in_port_t port, std::string serverName, std::string uri)
{
    *this = ConfigInfo();
    initConfigInfo(ip, port, serverName, uri);
}

ConfigInfo::~ConfigInfo()
{
}

ConfigInfo &ConfigInfo::operator=(const ConfigInfo &ConfigInfo)
{
    if (this == &ConfigInfo)
        return *this;
    _root = ConfigInfo._root;
    _cgiPath = ConfigInfo._cgiPath;
    _path = ConfigInfo._path;
    for (size_t i = 0; i < 4; i++)
        _allowMethods[i] = ConfigInfo._allowMethods[i];
    _index = ConfigInfo._index;
    _errorPage = ConfigInfo._errorPage;
    _autoIndex = ConfigInfo._autoIndex;
    _isRedirect = ConfigInfo._isRedirect;
    _requestTimeout = ConfigInfo._requestTimeout;
    _keepaliveTimeout = ConfigInfo._keepaliveTimeout;
    _redirect = ConfigInfo._redirect;
    _info = ConfigInfo._info;
    return *this;
}

// ip, port, serverName으로 서버 찾기
std::vector<ServerConfig>::iterator ConfigInfo::findMatchedServer(in_addr_t ip, in_port_t port, std::string serverName) {
    Config &config = Config::getInstance();
    std::vector<ServerConfig> &server_v = config.getServerConfig();
    std::vector<ServerConfig>::iterator it = server_v.begin();
    std::vector<ServerConfig>::iterator matchedServer;
    short matchedLevel = 0;
    for (; it != server_v.end(); it++) {
        if (matchedLevel < 2 && it->getIp().s_addr == 0 && it->getPort() == htons(port)) {
            if (it->getServerName() == serverName) {
                matchedServer = it;
                matchedLevel = 2;
            }
            else if (matchedLevel == 0) {
                matchedServer = it;
                matchedLevel = 1;
            }
        }
        if (it->getIp().s_addr == htonl(ip) && it->getPort() == htons(port)) {
            if (it->getServerName() == serverName) {
                matchedServer = it;
                matchedLevel = 4;
                break;
            }
            if (matchedLevel != 3) {
                matchedServer = it;
                matchedLevel = 3;
            }
        }
    }
    if (matchedLevel == 0)
        throw std::runtime_error("ConfigInfo 생성자에서 서버 찾기 실패: 매칭된 서버 없음");
    return matchedServer;
}

// 데이터 채우기
void ConfigInfo::transferInfo(t_directives &directives) {
    for (t_directives::iterator it = directives.begin(); it != directives.end(); it++) {
        if (it->first == "root") {
            if (it->second[0] == "html")
                _root = DEFAULT_ROOT;
            else {
                if (it->second[0][it->second[0].size() - 1] != '/')
                    _root = it->second[0] + '/';
                else
                    _root = it->second[0];
            }
        }
        else if (it->first == "autoindex") {
            if (it->second[0] == "on")
                _autoIndex = true;
            else if (it->second[0] == "off")
                _autoIndex = false;
            else
                throw std::runtime_error("ConfigInfo 생성자에서 autoindex 설정 실패");
        }
        else if (it->first == "index") {
            _index = it->second[0];
        }
        else if (it->first == "allow_method") {
            for (size_t i = 0; i < 4; i++)
                _allowMethods[i] = false;
            for (size_t i = 0; i < it->second.size(); i++) {
                if (it->second[i] == "GET")
                    _allowMethods[0] = true;
                else if (it->second[i] == "HEAD")
                    _allowMethods[1] = true;
                else if (it->second[i] == "POST")
                    _allowMethods[2] = true;
                else if (it->second[i] == "DELETE")
                    _allowMethods[3] = true;
                else
                    throw std::runtime_error("ConfigInfo 생성자에서 allow_methods 설정 실패");
            }
        }
        else if (it->first == "error_page") { // test 필요
            size_t sz = it->second.size();
            std::string route = it->second[sz - 1];
            for (size_t i = 0; i < sz - 1; i++) {
                _errorPage[it->second[i]] = route;
            }
        }
        else if (it->first == "return") { // 301 or 302이면 redirect 설정, 아니면 빈문자(나중에 에러처리용)
            _isRedirect = true;
            if (it->second.size() != 2 || (it->second[0] != "301" && it->second[0] != "302"))
                _redirect = std::make_pair("", "");
            else
                _redirect = std::make_pair(it->second[0], it->second[1]);
        }
        else if (it->first == "request_timeout") {
            if (it->second.size() == 1)
                _requestTimeout = stringToType<intptr_t>(it->second[0]);
        }
        else if (it->first == "keepalive_timeout") {
            if (it->second.size() == 1)
                _keepaliveTimeout = stringToType<intptr_t>(it->second[0]);
        }
        else {
            _info[it->first] = it->second;
        }
    }
}

// uri로 location 찾기
LocationConfig &ConfigInfo::findMatchedLocation(std::string &uri, std::map<std::string, LocationConfig> &locationMap, std::string &path) {
    // if (uri.find('.') != std::string::npos) {
    //     size_t pos = uri.find_last_of('/');
    //     if (uri.find('.') < pos)
    //         throw std::runtime_error("경로내에 .이 있습니다.");
    //     path = uri.substr(0, uri.find_last_of('/') + 1);
    // }
    if (uri[uri.size() - 1] == '/')
        path = uri;
    else
        path = uri + '/';
    while (locationMap.find(path) == locationMap.end()) {
        path = path.substr(0, path.find_last_of('/'));
        if (path == "")
            break;
        size_t pos = path.find_last_of('/');
        path = path.substr(0, pos + 1);
    }
    if (path == "")
        throw std::runtime_error("ConfigInfo 생성자에서 location 찾기 실패");
    return locationMap[path];
}

// matchedServer에서 먼저 데이터 넣고, matchedLocation에도 있으면 거기서 덮어씌우기
void ConfigInfo::initConfigInfo(in_addr_t ip, in_port_t port, std::string serverName, std::string uri) {
    ServerConfig &matchedServer = *findMatchedServer(ip, port, serverName);
    transferInfo(matchedServer.getServerInfo());
    // LocationConfig location 찾기
    std::string path;
    LocationConfig &matchedLocation = findMatchedLocation(uri, matchedServer.getLocationList(), path);
    transferInfo(matchedLocation.getLocationInfo());
    if (_root.back() == '/' && uri.front() == '/')
        _path = _root + uri.substr(1);
    else
        _path = _root + uri;

    t_directives::iterator it;
    std::string extension;
    size_t cgiPos;

    if ((it = _info.find("cgi")) != _info.end() && it->second.size() == 1) {
        extension = "." + it->second[0];
        if ((cgiPos = uri.find(extension, path.size())) != std::string::npos) {
            if (cgiPos + extension.size() >= uri.size() || uri[cgiPos + extension.size()] == '/') {
                _cgiPath = uri.substr(0, cgiPos + extension.size());
                _path = _root + uri.substr(_cgiPath.size() + 1);
            }
        }
    }
    if (_info.find("limit_client_body_size") == _info.end())
        _info["limit_client_body_size"].push_back("1000000");
}

// for test
void ConfigInfo::printConfigInfo() {
    std::cout << "root: " << _root << std::endl;
    std::cout << "path: " << _path << std::endl;
    std::string allowMethods[4] = { "GET", "HEAD", "POST", "DELETE" };
    std::cout << "allow_methods: ";
    for (size_t i = 0; i < 4; i++) {
        if (_allowMethods[i])
            std::cout << allowMethods[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "index: " << _index << std::endl;
    std::cout << "error_page" << std::endl;
    for (std::map<std::string, std::string>::iterator it = _errorPage.begin(); it != _errorPage.end(); it++) {
        std::cout << "num : " << it->first << ", route: " << it->second << std::endl;
    }
    std::cout << "autoindex: " << _autoIndex << std::endl;
    std::cout << "info: " << std::endl;
    for (t_directives::iterator it = _info.begin(); it != _info.end(); it++) {
        std::cout << it->first << ": ";
        for (size_t i = 0; i < it->second.size(); i++) {
            std::cout << it->second[i] << " ";
        }
        std::cout << std::endl;
    }
}

std::string ConfigInfo::getPrintableConfigInfo() {
    std::ostringstream result;

    result << "root: " << _root << "\n";
    result << "path: " << _path << "\n";

    std::string allowMethods[4] = { "GET", "HEAD", "POST", "DELETE" };
    result << "allow_methods: ";
    for (size_t i = 0; i < 4; i++) {
        if (_allowMethods[i] == true)
            result << allowMethods[i] << " ";
    }
    result << "\n";

    result << "index: " << _index << "\n";
    result << "error_page" << std::endl;
    for (std::map<std::string, std::string>::iterator it = _errorPage.begin(); it != _errorPage.end(); it++) {
        result << "num : " << it->first << ", route: " << it->second << std::endl;
    }
    result << "autoindex: " << _autoIndex << "\n";

    result << "\ninfo\n";
    for (t_directives::iterator it = _info.begin(); it != _info.end(); it++) {
        result << it->first << ": ";
        for (size_t i = 0; i < it->second.size(); i++) {
            result << it->second[i] << " ";
        }
        result << "\n";
    }

    // 문자열로 변환된 결과 반환
    return result.str();
}

// getter
std::string ConfigInfo::getRoot() const {
    return _root;
}

std::string ConfigInfo::getCgiPath() const {
    return _cgiPath;
}

std::string ConfigInfo::getPath() const {
    return _path;
}

bool ConfigInfo::getAllowMethods(int index) const {
    return _allowMethods[index];
}

std::string ConfigInfo::getIndex() const {
    return _index;
}

std::string ConfigInfo::getErrorPage(std::string key) const {
    if (_errorPage.find(key) == _errorPage.end()) {
        if (DEFAULT_PAGE.find(key) == DEFAULT_PAGE.end())
            return DEFAULT_PAGE.at("default");
            // throw std::runtime_error("ConfigInfo에서 errorPage 찾기 실패");
        return DEFAULT_PAGE.at(key);
    }
    return _errorPage.at(key);
}

std::string ConfigInfo::getServerName() const
{
    t_directives::const_iterator it;

    if ((it = _info.find("server_name")) != _info.end() && !it->second.empty())
        return it->second[0];
    return "";
}

bool ConfigInfo::getAutoIndex() const {
    return _autoIndex;
}

t_directives ConfigInfo::getInfo() const {
    return _info;
}

bool ConfigInfo::getIsRedirect() const {
    return _isRedirect;
}

intptr_t ConfigInfo::getRequestTimeout() const
{
    return _requestTimeout;
}

intptr_t ConfigInfo::getKeepaliveTimeout() const
{
    return _keepaliveTimeout;
}

std::pair<std::string, std::string> ConfigInfo::getRedirect() const {
    if (!_isRedirect)
        throw std::runtime_error("ConfigInfo에서 redirect 찾기 실패");
    return _redirect;
}

short ConfigInfo::requestType(HttpRequest& httpRequest) const
{
    if ((httpRequest.getCode() != 0) || _isRedirect || _cgiPath.empty())
        return MAKE_HTTP_RESPONSE;
    return MAKE_CGI_REQUEST;
}

void ConfigInfo::setDefaultErrorPage(unsigned short code) {
    
    _errorPage[toString(code)] = getDefaultPage(code);
}
