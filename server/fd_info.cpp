#include "fd_info.hpp"
#include "server_manager.hpp"

fd_info::fd_info()
: _type(server_manager::SERV_DEFAULT), _http(NULL)
{}

fd_info::fd_info(int type, http* http)
: _type(type), _http(http)
{}
