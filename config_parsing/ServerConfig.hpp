#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include <arpa/inet.h>
#include "LocationConfig.hpp"

typedef std::map<std::string, std::vector<std::string>> t_directives;

/*
** ServerConfig는 하나의 서버를 나타낸다.
*/
class ServerConfig
{
private:
    /* data */
    // server name이랑 listen은 따로 저장할까? DEFAULT 값도 설정하고
    struct in_addr ip;
    int port;
    std::vector<std::string> server_name;
    t_directives _server_info;
    std::map<std::string, LocationConfig> _location_m;

public:
    static std::vector<int> portsWithINADDR_ANY;

    ServerConfig();
    ServerConfig(const ServerConfig &ref);
    ~ServerConfig();

    ServerConfig &operator=(const ServerConfig &ref);

    // add int to portsWithINADDR_ANY
    static void addPort(int port);

    // getter
    t_directives &getServerInfo();
    std::vector<std::string> &getVariable(const std::string &key);
    struct in_addr getIp();
    int getPort();
    std::vector<std::string> getServerName();
    std::map<std::string, LocationConfig> &getLocationList();
    LocationConfig &getLocation(std::string &path);

    // setter
    void setServerInfo(t_directives &server_info);
    void setVariable(std::string &key, std::vector<std::string> &value);
    void setIp(struct in_addr ip);
    void setPort(int port);
    void setServerName(std::vector<std::string> &server_name);
    void setLocationList(std::map<std::string, LocationConfig> &location_m);
    void setLocation(std::string path, LocationConfig &location);
};

#endif
