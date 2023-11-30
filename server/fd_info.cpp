#include "fd_info.hpp"

fd_info::fd_info(const std::string& ip, u_short port, int type, http* http)
: _ip(ip), _port(port), _type(type), _http(http)
{
    if (this->_http == NULL)
        this->_http = new http();
}
