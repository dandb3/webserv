#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "../utils/FileReader.hpp"
#include "../utils/utils.hpp"
#include "Config.hpp"

class Config;

class ConfigParser
{
private:
    static const std::set<std::string> SERVER_KEY_SET;

    static bool isAlreadyExist(std::vector<ServerConfig> &serverList, ServerConfig &serverConfig);

    static void parseServer(std::string const &fileContent, size_t &i, Config &config);

    static std::pair<struct in_addr, int> getIpPort(std::string listen);

    static void parseLocation(std::string const &fileContent, size_t &i, ServerConfig &serverConfig, LocationConfig &locationConfig);

    static void parseTypes(std::string const &filePath, Config &config);

public:
    static void parse(std::string const &configPath, Config &config);
};

#endif
