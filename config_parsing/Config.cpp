#include <iostream>

#include "Config.hpp"

std::string Config::DEFAUT_PATH = "./config/default.conf";

Config::Config() {
    // parseConfig(DEFAUT_PATH);
    std::cout << "default_path: " << DEFAUT_PATH << std::endl;
    ConfigParser::parse(DEFAUT_PATH, *this);
    std::cout << "parse finish" << std::endl;
}


Config::Config(std::string const& config_path) {
    // parseConfig(config_path);
    std::cout << "config_path: " << config_path << std::endl;
    ConfigParser::parse(config_path, *this);
    std::cout << "parse finish" << std::endl;
}

Config::~Config() {
}

// 구현 x
Config::Config(Config const& ref) {}
Config& Config::operator=(Config const& ref) {
    return *this;
}

// getInstace
Config& Config::getInstance() {
    static Config _instance(DEFAUT_PATH);
    return _instance;
}

Config& Config::getInstance(std::string const& config_path) {
    static Config _instance(config_path);
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