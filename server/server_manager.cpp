#include <set>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server_manager.hpp"
#include "Exception.hpp"

server_manager::server_manager(const char* path)
: _conf(path)
{
    const std::vector<server>& serverv = _conf.get_servers();
    std::set<std::pair<uint32_t, unsigned short> > occupied;

    for (size_t i = 0; i < serverv.size(); ++i)
        if (serverv[i].get_ip() == INADDR_ANY)
            occupied.insert(std::make_pair(INADDR_ANY, serverv[i].get_port()));
    for (size_t i = 0; i < serverv.size(); ++i)
        if (occupied.find(std::make_pair(INADDR_ANY, serverv[i].get_port())) == occupied.end())
            occupied.insert(std::make_pair(serverv[i].get_ip(), serverv[i].get_port()));

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
        _handler.add_type(sockfd, SERV_LISTEN);
        _handler.event_update(sockfd, EVFILT_READ, EV_ADD);
        _fd_info_m.insert(std::make_pair(sockfd, NULL));
    }
}

void server_manager::serv_start()
{
    while (true) {
        _handler.event_catch();
        for (int i = 0; i < _handler.get_nevents(); ++i) {
            switch (_handler.event_type(i)) {
            case SERV_LISTEN:
                _sock_listen();
                break;
            case SERV_HTTP_REQ:
                _http_request();
                break;
            case SERV_HTTP_RES:
                _http_response();
                break;
            case SERV_CGI_REQ:
                _cgi_request();
                break;
            case SERV_CGI_RES:
                _cgi_response();
                break;
            case SERV_ERROR:
                _serv_error();
                break;
            }
        }
    }
}
