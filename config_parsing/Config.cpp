#include <iostream>

#include "Config.hpp"

std::string Config::DEFAULT_PATH = "./conf/default.conf";

Config Config::_instance = Config();

Config::Config() {
    _config_path = "";
}

Config::Config(std::string const& config_path) {
    // parseConfig(config_path);
    std::cout << "config_path: " << config_path << std::endl;
    ConfigParser::parse(config_path, *this);
    std::cout << "parse finish" << std::endl;
    _config_path = config_path;
}

Config::~Config() {
}

Config::Config(const Config& ref) {
    // 복사 생성자의 구현: 적절히 멤버 변수를 복사하여 새로운 객체 생성
    _http_info = ref._http_info;
    _server_v = ref._server_v;
    _mime_types = ref._mime_types;
    _config_path = ref._config_path;
}

Config& Config::operator=(const Config& ref) {
    // 대입 연산자의 구현: 적절히 멤버 변수를 복사하여 현재 객체에 대입
    if (this != &ref) {
        _http_info = ref._http_info;
        _server_v = ref._server_v;
        _mime_types = ref._mime_types;
        _config_path = ref._config_path;
    }
    return *this;
}


// getInstace
Config& Config::getInstance() {
    if (_instance._config_path == "")
        _instance = Config(DEFAULT_PATH);
    return _instance;
}

Config& Config::getInstance(std::string const& config_path) {
    if (_instance._config_path == "")
        _instance = Config(config_path);
    return _instance;
}

// getter
t_directives& Config::getHttpInfo() {
    return this->_http_info;
}

std::vector<std::string>& Config::getVariable(std::string& key) {
    return this->_http_info[key];
}

std::vector<ServerConfig>& Config::getServerConfig() {
    return this->_server_v;
}

t_directives& Config::getMimeTypes() {
    return this->_mime_types;
}

// setter
void Config::setHttpInfo(t_directives& http_info) {
    this->_http_info = http_info;
}

void Config::setVariable(std::string& key, std::vector<std::string>& value) {
    this->_http_info[key] = value;
}

void Config::setServerConfig(std::vector<ServerConfig>& server_v) {
    this->_server_v = server_v;
}

void Config::setServer(ServerConfig& server) {
    // 똑같은 서버가 있는지 확인 과정 필요
    this->_server_v.push_back(server);
}

void Config::setMimeTypes(t_directives& mime_types) {
    this->_mime_types = mime_types;
}
