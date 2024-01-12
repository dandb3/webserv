#include <iostream>

#include "ServerConfig.hpp"

std::vector<int> ServerConfig::portsWithINADDR_ANY;

ServerConfig::ServerConfig() {
}

ServerConfig::ServerConfig(ServerConfig const &ref) {
    *this = ref;
}

ServerConfig::~ServerConfig() {
}

ServerConfig &ServerConfig::operator=(ServerConfig const &ref) {
    if (this != &ref) {
        this->_server_info = ref._server_info;
        this->_location_m = ref._location_m;
        this->ip = ref.ip;
        this->port = ref.port;
        this->server_name = ref.server_name;
    }
    return *this;
}

// add int to portsWithINADDR_ANY
void ServerConfig::addPort(int port) {
    std::vector<int>::iterator it = std::find(ServerConfig::portsWithINADDR_ANY.begin(), ServerConfig::portsWithINADDR_ANY.end(), port);
    if (it == ServerConfig::portsWithINADDR_ANY.end())
        ServerConfig::portsWithINADDR_ANY.push_back(port);
}

// getter
t_directives &ServerConfig::getServerInfo() {
    return this->_server_info;
}

std::vector<std::string> &ServerConfig::getVariable(const std::string &key) {
    return this->_server_info[key];
}

std::map<std::string, LocationConfig> &ServerConfig::getLocationList() {
    return this->_location_m;
}

struct in_addr ServerConfig::getIp() {
    return this->ip;
}

int ServerConfig::getPort() {
    return this->port;
}

std::string ServerConfig::getServerName() {
    return this->server_name;
}

LocationConfig &ServerConfig::getLocation(std::string &path) {
    if (this->_location_m.find(path) != this->_location_m.end())
        return this->_location_m[path];
    throw std::runtime_error("location이 존재하지 않습니다.");
}

// setter
void ServerConfig::setServerInfo(t_directives &server_info) {
    this->_server_info = server_info;
}

void ServerConfig::setVariable(std::string &key, std::vector<std::string> &value) {
    this->_server_info[key] = value;
}

void ServerConfig::setIp(struct in_addr ip) {
    this->ip = ip;
}

void ServerConfig::setPort(int port) {
    this->port = port;
}

void ServerConfig::setServerName(std::string &server_name) {
    this->server_name = server_name;
}

void ServerConfig::setLocationList(std::map<std::string, LocationConfig> &location_m) {
    this->_location_m = location_m;
}

void ServerConfig::setLocation(std::string path, LocationConfig &location) {
    this->_location_m.insert(std::pair<std::string, LocationConfig>(path, location));
}
