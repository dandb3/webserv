#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include "ServerConfig.hpp"

typedef std::map<std::string, std::vector<std::string>> t_directives;

// singleton
class Config
{
private:
    /* data */
    t_directives _http_info;
    std::vector<ServerConfig> _server_v;
    t_directives _mime_types; // 나중에 어떻게 사용하는지 보고 자료구조 변경할 듯

    static std::string DEFAUT_PATH;

    /* private functions */
    Config();
    Config(const Config &ref); // x
    Config(std::string const &config_path);
    ~Config();

    Config &operator=(const Config &ref); // x

    static Config _instance;

public:
    // void parseConfig(std::string const &config_path);
    static Config &getInstance();
    static Config &getInstance(std::string const &config_path);

    // getter
    t_directives &getHttpInfo();
    std::vector<std::string> &getVariable(std::string &key);

    std::vector<ServerConfig> &getServerConfig();
    t_directives &getMimeTypes();

    // setter
    void setHttpInfo(t_directives &http_info);
    void setVariable(std::string &key, std::vector<std::string> &value);

    void setServerConfig(std::vector<ServerConfig> &server_v);
    void setMimeTypes(t_directives &mime_types);
};

#endif
