#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "utils/FileReader.hpp"
#include "utils/utils.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "Config.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class Config;

class ConfigParser
{
private:
    static void parseServer(std::string const &file_content, size_t &i, Config &config);

    static std::pair<struct in_addr, int> getIpPort(std::string listen);

    static void parseLocation(std::string const &file_content, size_t &i, ServerConfig &server_config, LocationConfig &location_config);

    static void parseTypes(std::string const &file_path, Config &config);

public:
    static void parse(std::string const &config_path, Config &config);
};

#endif
