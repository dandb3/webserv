#include <set>
#include <sys/socket.h>
#include "server_manager.hpp"
#include "Exception.hpp"

server_manager::server_manager(const char* path)
: _conf(path), _fd_infov(3), _handler()
{
    const std::vector<server>& serverv = _conf.get_servers();
    std::set<std::pair<std::string, unsigned short> > owned;

    for (size_t i = 0; i < serverv.size(); ++i)
        if (serverv[i].get_ip() == INADDR_ANY_STR)
            owned.insert({INADDR_ANY_STR, serverv[i].get_port()});
    for (size_t i = 0; i < serverv.size(); ++i)
        if (owned.find({INADDR_ANY_STR, serverv[i].get_port()}) == owned.end())
            owned.insert({serverv[i].get_ip(), serverv[i].get_port()});

    int sockfd;
    struct sockaddr_in sockaddr;
    

    for (std::set<std::pair<std::string, unsigned short> >::iterator it = owned.begin();
        it != owned.end(); ++it) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw err_syscall();
        if (bind(sockfd, ))
    }
}
