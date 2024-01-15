#include <iostream>

#include "Config.hpp"

std::string Config::DEFAULT_PATH = "confTest/default.conf";

Config Config::_instance = Config();

Config::Config() {
    _configPath = "";
}

Config::Config(std::string const &configPath) {
    // parseConfig(configPath);
    std::cout << "configPath: " << configPath << std::endl;
    ConfigParser::parse(configPath, *this);
    std::cout << "parse finish" << std::endl;
    _configPath = configPath;
}

Config::~Config() {
}

Config::Config(const Config &ref) {
    // 복사 생성자의 구현: 적절히 멤버 변수를 복사하여 새로운 객체 생성
    _httpInfo = ref._httpInfo;
    _serverList = ref._serverList;
    _mimeTypes = ref._mimeTypes;
    _configPath = ref._configPath;
}

Config &Config::operator=(const Config &ref) {
    // 대입 연산자의 구현: 적절히 멤버 변수를 복사하여 현재 객체에 대입
    if (this != &ref) {
        _httpInfo = ref._httpInfo;
        _serverList = ref._serverList;
        _mimeTypes = ref._mimeTypes;
        _configPath = ref._configPath;
    }
    return *this;
}

// getInstace
Config &Config::getInstance() {
    if (_instance._configPath == "")
        _instance = Config(DEFAULT_PATH);
    return _instance;
}

Config &Config::getInstance(std::string const &configPath) {
    if (_instance._configPath == "")
        _instance = Config(configPath);
    return _instance;
}

// getter
t_directives &Config::getHttpInfo() {
    return this->_httpInfo;
}

std::vector<std::string> &Config::getVariable(std::string &key) {
    return this->_httpInfo[key];
}

std::vector<ServerConfig> &Config::getServerConfig() {
    return this->_serverList;
}

t_directives &Config::getMimeTypes() {
    return this->_mimeTypes;
}

// setter
void Config::setHttpInfo(t_directives &httpInfo) {
    this->_httpInfo = httpInfo;
}

void Config::setVariable(std::string &key, std::vector<std::string> &value) {
    this->_httpInfo[key] = value;
}

void Config::setServerConfig(std::vector<ServerConfig> &serverList) {
    this->_serverList = serverList;
}

void Config::setServer(ServerConfig &server) {
    // 똑같은 서버가 있는지 확인 과정 필요
    this->_serverList.push_back(server);
}

void Config::setMimeTypes(t_directives &mimeTypes) {
    this->_mimeTypes = mimeTypes;
}
