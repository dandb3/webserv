#include <iostream>

#include "LocationConfig.hpp"

LocationConfig::LocationConfig() {
}

LocationConfig::LocationConfig(LocationConfig const &ref) {
    *this = ref;
}

LocationConfig::~LocationConfig() {
}

LocationConfig &LocationConfig::operator=(LocationConfig const &ref) {
    if (this != &ref) {
        this->_locationInfo = ref._locationInfo;
    }
    return *this;
}

// getter
t_directives &LocationConfig::getLocationInfo() {
    return this->_locationInfo;
}

std::vector<std::string> &LocationConfig::getVariable(std::string &key) {
    return this->_locationInfo[key];
}

// setter
void LocationConfig::setLocationInfo(t_directives &location_info) {
    this->_locationInfo = location_info;
}

void LocationConfig::setVariable(std::string &key, std::vector<std::string> &value) {
    this->_locationInfo[key] = value;
}
