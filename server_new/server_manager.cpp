#include <set>
#include <string.h>
#include <netinet/in.h>
#include "server_manager.hpp"

server_manager::server_manager(const char* path)
: _conf(path), _get_v(GETV_SIZE)
{
    const std::vector<server>& server_v = _conf.get_servers();
    std::set<std::pair<uint32_t, unsigned short> > occupied;

    if ((_kq = kqueue()) == -1)
        throw err_syscall();
    for (size_t i = 0; i < server_v.size(); ++i)
        if (server_v[i].get_ip() == INADDR_ANY)
            occupied.insert(std::make_pair(INADDR_ANY, server_v[i].get_port()));
    for (size_t i = 0; i < server_v.size(); ++i)
        if (occupied.find(std::make_pair(INADDR_ANY, server_v[i].get_port())) == occupied.end())
            occupied.insert(std::make_pair(server_v[i].get_ip(), server_v[i].get_port()));

    int sockfd;
    struct sockaddr_in sock_addr;
    std::set<std::pair<uint32_t, unsigned short> >::iterator it;

    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    for (it = occupied.begin(); it != occupied.end(); ++it) {
        sock_addr.sin_addr.s_addr = htonl(it->first);
        sock_addr.sin_port = htons(it->second);
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw err_syscall();
        if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&sockaddr), sizeof(struct sockaddr_in)) == -1)
            throw err_syscall();
        _fd_info_m.insert(std::make_pair(sockfd, fd_info(fd_info::SERV_LISTEN, NULL)));
        _event_update(sockfd, EVFILT_READ, EV_ADD);
    }
}

void server_manager::_event_catch()
{
    _nevents = kevent(_kq, (_set_v.size() ? &_set_v[0] : NULL),
        _set_v.size(), (_get_v.size() ? &_get_v[0] : NULL),
        _get_v.size(), &_timeout);
    if (_nevents == -1)
        throw err_syscall();
    _set_v.clear();
}

void server_manager::_event_update(uintptr_t fd, short filter, u_short flags)
{
    struct kevent new_event;

    EV_SET(&new_event, fd, filter, flags, 0, 0, NULL);
    _set_v.push_back(new_event);
}

void server_manager::_serv_listen(struct kevent& kev)
{
    int new_sockfd;

    new_sockfd = accept(kev.ident, NULL, NULL);
    _fd_info_m[new_sockfd] = fd_info(fd_info::SERV_HTTP_REQ, new info(new_sockfd));
    _event_update(new_sockfd, EVFILT_READ, EV_ADD);
}

void server_manager::_serv_http_request(struct kevent& kev)
{
    
}

void server_manager::operate()
{
    while (true) {
        _event_catch();
        for (int i = 0; i < _nevents; ++i) {
            switch(_event_type(_get_v[i].ident)) {
            case fd_info::SERV_LISTEN:
                break;
            case fd_info::SERV_HTTP_REQ:
                break;
            case fd_info::SERV_HTTP_RES:
                break;
            case fd_info::SERV_CGI_REQ:
                break;
            case fd_info::SERV_CGI_RES:
                break;
            case fd_info::SERV_ERROR:
                break;
            }
        }
    }
}
