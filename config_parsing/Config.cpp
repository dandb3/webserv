#include <iostream>

#include "Config.hpp"

std::string Config::DEFAUT_PATH = "./config/default.conf";

Config::Config() {
    // parseConfig(DEFAUT_PATH);
}


Config::Config(std::string const& config_path) {
    // parseConfig(config_path);
}

Config::~Config() {
}

// 구현 x
Config::Config(Config const& ref) {}
Config& Config::operator=(Config const& ref) {}

// getInstace
Config& Config::getInstance() {
    if (&Config::_instance == NULL)
        Config::_instance = Config();
    return Config::_instance;
}

Config& Config::getInstance(std::string const& config_path) {
    if (&Config::_instance == NULL)
        Config::_instance = Config(config_path);
    return Config::_instance;
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

void Config::setMimeTypes(t_directives& mime_types) {
    this->_mime_types = mime_types;
}