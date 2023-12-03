#include "fd_info.hpp"
#include "server_manager.hpp"

fd_info::fd_info()
: _ip(), _port(0), _type(server_manager::SERV_DEFAULT), _http(NULL)
{}

fd_info::fd_info(const std::string& ip, u_short port, int type, http* http)
: _ip(ip), _port(port), _type(type), _http(http)
{
    if (_http == NULL)
        _http = new http();
}
