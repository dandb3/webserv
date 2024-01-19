#include "ConfigInfo.hpp"
#include <sstream>

const std::string ConfigInfo::DEFAULT_INDEX = "index.html";
const std::string ConfigInfo::DEFAULT_ROOT = "/var/www/html";
const std::string ConfigInfo::DEFAULT_ERROR_PAGE = "/confTest/error/defaultError.html";

/// @brief ip, port를 보고 matchedServer,
///        uri를 보고 matchedLocation을 찾아서 ConfigInfo 생성
ConfigInfo::ConfigInfo()
{
    // ip, port을 보고 match되는 config를 찾아서 생성자 호출
    _root = DEFAULT_ROOT;
    for (size_t i = 0; i < 4; i++)
        _allowMethods[i] = false;
    _index = DEFAULT_INDEX;
    _errorPage = DEFAULT_ERROR_PAGE;
    _autoIndex = false;
    _info.clear();
    _path = "";
}

ConfigInfo::ConfigInfo(in_addr_t ip, in_port_t port, std::string uri)
{
    *this = ConfigInfo();
    initConfigInfo(ip, port, uri);
}

ConfigInfo::~ConfigInfo()
{
}

ConfigInfo &ConfigInfo::operator=(const ConfigInfo &ConfigInfo)
{
    if (this == &ConfigInfo)
        return *this;
    _root = ConfigInfo._root;
    _path = ConfigInfo._path;
    for (size_t i = 0; i < 4; i++)
        _allowMethods[i] = ConfigInfo._allowMethods[i];
    _index = ConfigInfo._index;
    _autoIndex = ConfigInfo._autoIndex;
    _info = ConfigInfo._info;
    return *this;
}

// matchedServer에서 먼저 데이터 넣고, matchedLocation에도 있으면 거기서 덮어씌우기
void ConfigInfo::initConfigInfo(in_addr_t ip, in_port_t port, std::string uri) {
    Config &config = Config::getInstance();
    std::vector<ServerConfig> &server_v = config.getServerConfig();
    std::vector<ServerConfig>::iterator it = server_v.begin();
    ServerConfig matchedServer;
    bool isMatched = false;
    // ip, port로 서버 찾기
    for (; it != server_v.end(); it++) {
        if (it->getIp().s_addr == 0 && it->getPort() == port) {
            matchedServer = *it;
            isMatched = true;
        }
        if (it->getIp().s_addr == ip && it->getPort() == port) {
            matchedServer = *it;
            isMatched = true;
            break;
        }
    }
    if (!isMatched)
        throw std::runtime_error("ConfigInfo 생성자에서 서버 찾기 실패");
    t_directives &serverInfo = matchedServer.getServerInfo();
    for (t_directives::iterator it = serverInfo.begin(); it != serverInfo.end(); it++) {
        if (it->first == "root") {
            _root = it->second[0];
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
        else {
            _info[it->first] = it->second;

        }
    }
    // LocationConfig location 찾기
    std::map<std::string, LocationConfig> &locationMap = matchedServer.getLocationList();
    std::string dir = uri;
    while (locationMap.find(dir) == locationMap.end()) {
        if (dir == "/")
            break;
        size_t pos = dir.find_last_of('/');
        dir = pos == 0 ? "/" : uri.substr(0, pos);
    }
    if (locationMap.find(dir) == locationMap.end())
        throw std::runtime_error("ConfigInfo 생성자에서 location 찾기 실패");
    // dir 뒷부분
    std::string locationUri = (dir == "/") ? uri : uri.substr(dir.size());
    LocationConfig matchedLocation = locationMap[dir];
    t_directives &locationInfo = matchedLocation.getLocationInfo();
    for (t_directives::iterator it = locationInfo.begin(); it != locationInfo.end(); it++) {
        if (it->first == "root") {
            _root = it->second[0];
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
            for (size_t i = 0; i < 4; i++) {
                _allowMethods[i] = false;
            }
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
        else {
            _info[it->first] = it->second;
        }
    }
    if (_root[_root.size() - 1] == '/')
        _path = _root + locationUri.substr(1);
    else
        _path = _root + locationUri;
}

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
    std::cout << "error_page: " << _errorPage << std::endl;
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
    result << "error_page: " << _errorPage << "\n";
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

std::string ConfigInfo::getPath() const {
    return _path;
}

bool ConfigInfo::getAllowMethods(int index) const {
    return _allowMethods[index];
}

std::string ConfigInfo::getIndex() const {
    return _index;
}

std::string ConfigInfo::getErrorPage() const {
    return _errorPage;
}

bool ConfigInfo::getAutoIndex() const {
    return _autoIndex;
}

t_directives ConfigInfo::getInfo() const {
    return _info;
}

