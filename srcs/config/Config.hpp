#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "ServerConfig.hpp"
#include "ConfigParser.hpp"

class ConfigParser;

typedef std::map<std::string, std::vector<std::string> > t_directives; // map<simple directive>

// singleton
class Config
{
private:
    /* data */
    t_directives _httpInfo;
    std::vector<ServerConfig> _serverList;
    std::map<std::string, std::string> _mimeTypes; // 나중에 어떻게 사용하는지 보고 자료구조 변경할 듯

    std::string _configPath;

    static std::string DEFAULT_PATH;

    /* private functions */
    Config();
    Config(const Config &ref); // x
    Config(std::string const &configPath);
    ~Config();

    Config &operator=(const Config &ref); // x

    static Config _instance;

public:
    static Config &getInstance();
    static Config &getInstance(std::string const &configPath);

    // getter
    t_directives &getHttpInfo();
    std::vector<std::string> &getVariable(std::string &key);

    std::vector<ServerConfig> &getServerConfig();

    std::map<std::string, std::string> &getMimeTypes();

    // setter
    void setHttpInfo(t_directives &httpInfo);
    void setVariable(std::string &key, std::vector<std::string> &value);

    void setServerConfig(std::vector<ServerConfig> &serverList);
    void setServer(ServerConfig &server);
    void setMimeTypes(std::map<std::string, std::string> &mimeTypes);
};

#endif
