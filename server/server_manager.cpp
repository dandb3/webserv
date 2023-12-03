#include "server_manager.hpp"

server_manager::server_manager(const char* path)
: _conf(path), _handler(_conf), fd_infos(3)
{
    const std::vector<server>& servers = _conf.get_servers();

    for (size_t i = 0; i < servers.size(); ++i) {
        
    }
}
