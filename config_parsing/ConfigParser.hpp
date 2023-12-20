#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#define WHITESPACE " \t\n\r\f\v"
#include "utils/FileReader.hpp"
#include "Config.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

class ConfigParser
{
private:
public:
    ConfigParser();
    ConfigParser(const ConfigParser &ref);
    ~ConfigParser();

    ConfigParser &operator=(const ConfigParser &ref);
    void parseConfig(std::string const &config_path, Config &config);
};

#endif
