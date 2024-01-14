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
        this->_serverInfo = ref._serverInfo;
        this->_locationMap = ref._locationMap;
        this->_ip = ref._ip;
        this->_port = ref._port;
        this->_serverName = ref._serverName;
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
    return this->_serverInfo;
}

std::vector<std::string> &ServerConfig::getVariable(const std::string &key) {
    return this->_serverInfo[key];
}

std::map<std::string, LocationConfig> &ServerConfig::getLocationList() {
    return this->_locationMap;
}

struct in_addr ServerConfig::getIp() {
    return this->_ip;
}

int ServerConfig::getPort() {
    return this->_port;
}

std::string ServerConfig::getServerName() {
    return this->_serverName;
}

LocationConfig &ServerConfig::getLocation(std::string &path) {
    if (this->_locationMap.find(path) != this->_locationMap.end())
        return this->_locationMap[path];
    throw std::runtime_error("location이 존재하지 않습니다.");
}

// setter
void ServerConfig::setServerInfo(t_directives &serverInfo) {
    this->_serverInfo = serverInfo;
}

void ServerConfig::setVariable(std::string &key, std::vector<std::string> &value) {
    this->_serverInfo[key] = value;
}

void ServerConfig::setIp(struct in_addr ip) {
    this->_ip = ip;
}

void ServerConfig::setPort(int port) {
    this->_port = port;
}

void ServerConfig::setServerName(std::string &serverName) {
    this->_serverName = serverName;
}

void ServerConfig::setLocationList(std::map<std::string, LocationConfig> &locationMap) {
    this->_locationMap = locationMap;
}

void ServerConfig::setLocation(std::string path, LocationConfig &location) {
    this->_locationMap.insert(std::pair<std::string, LocationConfig>(path, location));
}
