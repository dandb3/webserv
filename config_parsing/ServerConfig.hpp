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
    struct sockaddr_in ip;
    int port;
    std::vector<std::string> server_name;
    t_directives _server_info;
    std::vector<LocationConfig> _location_v;

public:
    ServerConfig();
    ServerConfig(const ServerConfig &ref);
    ~ServerConfig();

    ServerConfig &operator=(const ServerConfig &ref);

    // getter
    t_directives &getServerInfo();
    std::vector<std::string> &getVariable(const std::string &key);
    struct sockaddr_in getIp();
    int getPort();
    std::vector<std::string> getServerName();

    std::vector<LocationConfig> &getLocationList();
    LocationConfig &getLocation(std::string &path);

    // setter
    void setServerInfo(t_directives &server_info);
    void setVariable(std::string &key, std::vector<std::string> &value);
    void setIp(struct sockaddr_in ip);
    void setPort(int port);
    void setServerName(std::vector<std::string> &server_name);
    void setLocationList(std::vector<LocationConfig> &location_v);
    void setLocation(LocationConfig &location);
};

#endif
