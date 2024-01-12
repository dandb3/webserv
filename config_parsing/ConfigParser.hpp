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
    static bool isAlreadyExist(std::vector<ServerConfig> &serverList, ServerConfig &serverConfig);

    static void parseServer(std::string const &fileContent, size_t &i, Config &config);

    static std::pair<struct in_addr, int> getIpPort(std::string listen);

    static void parseLocation(std::string const &fileContent, size_t &i, ServerConfig &serverConfig, LocationConfig &locationConfig);

    static void parseTypes(std::string const &filePath, Config &config);

public:
    static void parse(std::string const &configPath, Config &config);
};

#endif
