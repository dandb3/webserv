#include <iostream>

#include "LocationConfig.hpp"

LocationConfig::LocationConfig() {
}

LocationConfig::LocationConfig(LocationConfig const& ref) {
    *this = ref;
}

LocationConfig::~LocationConfig() {
}

LocationConfig& LocationConfig::operator=(LocationConfig const& ref) {
    if (this != &ref) {
        this->_path = ref._path;
        this->_location_info = ref._location_info;
    }
    return *this;
}

// getter
std::string& LocationConfig::getPath() {
    return this->_path;
}

t_directives& LocationConfig::getLocationInfo() {
    return this->_location_info;
}

std::vector<std::string>& LocationConfig::getVariable(std::string& key) {
    return this->_location_info[key];
}

// setter
void LocationConfig::setPath(std::string& path) {
    this->_path = path;
}

void LocationConfig::setLocationInfo(t_directives& location_info) {
    this->_location_info = location_info;
}

void LocationConfig::setVariable(std::string& key, std::vector<std::string>& value) {
    this->_location_info[key] = value;
}