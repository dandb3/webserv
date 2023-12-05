#include <set>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server_manager.hpp"
#include "Exception.hpp"

server_manager::server_manager(const char* path)
: _conf(path), _fd_infov(3), _handler()
{
    const std::vector<server>& serverv = _conf.get_servers();
    std::set<std::pair<uint32_t, unsigned short> > owned;

    for (size_t i = 0; i < serverv.size(); ++i)
        if (serverv[i].get_ip() == INADDR_ANY)
            owned.insert(std::make_pair(INADDR_ANY, serverv[i].get_port()));
    for (size_t i = 0; i < serverv.size(); ++i)
        if (owned.find(std::make_pair(INADDR_ANY, serverv[i].get_port())) == owned.end())
            owned.insert(std::make_pair(serverv[i].get_ip(), serverv[i].get_port()));

    int sockfd;
    struct sockaddr_in sock_addr;

    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    for (std::set<std::pair<uint32_t, unsigned short> >::iterator it = owned.begin();
        it != owned.end(); ++it) {
        sock_addr.sin_addr.s_addr = htonl((*it).first);
        sock_addr.sin_port = htons((*it).second);
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw err_syscall();
        if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&sockaddr), sizeof(struct sockaddr_in)) == -1)
            throw err_syscall();
        _add_elt(sockfd, SERV_LISTEN, NULL);
        _handler.event_update(sockfd, EVFILT_READ, EV_ADD);
    }
}

void server_manager::_add_elt(int fd, int type, http* http)
{
    if (_fd_infov.size() > fd)
        _fd_infov[fd] = fd_info(type, http);
    else
        _fd_infov.push_back(fd_info(type, http));
}

void server_manager::serv_start()
{
    while (true) {
        _handler.event_catch();
        for (int i = 0; i < _handler.get_nevents(); ++i) {
            switch (_type(i)) {
            case SERV_LISTEN:
                break;
            case SERV_HTTP_REQ:
                break;
            case SERV_HTTP_RES:
                break;
            case SERV_CGI_REQ:
                break;
            case SERV_CGI_RES:
                break;
            case SERV_ERROR:
                break;
            }
        }
    }
}
