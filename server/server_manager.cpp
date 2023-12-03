#include <set>
#include "server_manager.hpp"

server_manager::server_manager(const char* path)
: _conf(path), _fd_infov(3), _handler()
{
    std::set<std::pair<std::string, unsigned short> > unavail_sock;

    for (size_t i = 0; i < _conf.get_servers().size(); ++i) {
        _conf.get_servers()[i]
    }
}
