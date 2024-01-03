#include <iostream>

#include "ServerConfig.hpp"

ServerConfig::ServerConfig() {
}

ServerConfig::ServerConfig(ServerConfig const& ref) {
    *this = ref;
}

ServerConfig::~ServerConfig() {
}

ServerConfig& ServerConfig::operator=(ServerConfig const& ref) {
    if (this != &ref) {
        this->_server_info = ref._server_info;
        this->_location_v = ref._location_v;
    }
    return *this;
}

// getter
t_directives& ServerConfig::getServerInfo() {
    return this->_server_info;
}

std::vector<std::string>& ServerConfig::getVariable(const std::string& key) {
    return this->_server_info[key];
}

std::vector<LocationConfig>& ServerConfig::getLocationList() {
    return this->_location_v;
}

LocationConfig& ServerConfig::getLocation(std::string& path) {
    for (std::vector<LocationConfig>::iterator it = this->_location_v.begin(); it != this->_location_v.end(); it++) {
        if (it->getPath() == path)
            return *it;
    }
    return *this->_location_v.end();
}

// setter
void ServerConfig::setServerInfo(t_directives& server_info) {
    this->_server_info = server_info;
}

void ServerConfig::setVariable(std::string& key, std::vector<std::string>& value) {
    this->_server_info[key] = value;
}

void ServerConfig::setLocationList(std::vector<LocationConfig>& location_v) {
    this->_location_v = location_v;
}

void ServerConfig::setLocation(LocationConfig& location) {
    this->_location_v.push_back(location);
}
