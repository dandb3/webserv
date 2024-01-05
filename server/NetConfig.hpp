#ifndef NETCONFIG_HPP
# define NETCONFIG_HPP

# include <netinet/in.h>
# include "Config.hpp"

// int 자료형을 쓸 것인지, string 자료형을 쓸 것인지, 인자로 어떤 자료형을 받을 것인지, network byte order를 따를지 등은 논의 필요.
// 사실 얘는 그냥 public 멤버 변수들로만 구성해도 괜찮을 것 같은데..

class NetConfig
{
private:
    uint32_t _ip;
    u_short _port;
    /**
     * root
     * error_page
     * autoindex
     * index
     * etc...
    */

public:

    NetConfig();

    inline uint32_t getIp() const;
    inline u_short getPort() const;

    inline void setIp(uint32_t ip);
    inline void setPort(u_short port);

};

inline uint32_t NetConfig::getIp() const
{
    return _ip;
}

inline u_short NetConfig::getPort() const
{
    return _port;
}

inline void NetConfig::setIp(uint32_t ip)
{
    _ip = ip;
}

inline void NetConfig::setPort(u_short port)
{
    _port = port;
}

#endif
