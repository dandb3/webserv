#include <set>
#include <queue>
#include <string.h>
#include <netinet/in.h>
#include "server_manager.hpp"
#include "net_config.hpp"

server_manager::server_manager()
: _conf(Config::getInstance())
{
    const std::vector<ServerConfig>& server_v = _conf.getServerConfig();
    std::set<std::pair<uint32_t, unsigned short> > sockets;

    for (size_t i = 0; i < server_v.size(); ++i) {

    }

    const std::vector<server>& server_v = _conf.get_servers();
    std::set<std::pair<uint32_t, unsigned short> > occupied;

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

int server_manager::_get_type(const struct kevent& kev)
{
    if (kev.flags & EV_ERROR)
        return EVENT_ERROR;
	switch (_type_m[kev.ident]) {
	case SOCKET_LISTEN:
		return EVENT_LISTEN;
	case SOCKET_HTTP:
		if (kev.flags & EVFILT_READ)
			return EVENT_HTTP_REQ;
		else if (kev.flags & EVFILT_WRITE)
			return EVENT_HTTP_RES;
		else
			return EVENT_ERROR;
	case SOCKET_CGI:
		if (kev.flags & EVFILT_READ)
			return EVENT_CGI_REQ;
		else if (kev.flags & EVFILT_WRITE)
			return EVENT_CGI_RES;
		else
			return EVENT_ERROR;
	}
}

void server_manager::_serv_listen(const struct kevent& kev)
{
    int new_sockfd;

    new_sockfd = accept(kev.ident, NULL, NULL);
    _handler.ev_update(new_sockfd, EVFILT_READ, EV_ADD);
    _type_m.insert(std::make_pair(new_sockfd, SOCKET_HTTP));
    _http_request_m.insert(std::make_pair(new_sockfd, http_request(new_sockfd)));
}

void server_manager::_serv_http_request(const struct kevent& kev)
{
    const http_request& hreq = _http_request_m[kev.ident];

    hreq.recv_request(kev.data);
    hreq.parse_request();

    // Keep-Alive: change if -> while
    if (hreq.parsed()) {
        net_config nconf(hreq, _conf);
        /**
         * select server and location (by listen, Host, uri)
         * if (cgi_request) -> make_cgi_request();
         * else if (http_response) -> make_http_response();
        */
        hreq.reset_parsed();
    }
}

void server_manager::_serv_http_response(struct kevent& kev)
{
    const http_response& hres = _http_response_m[kev.ident];

    hres.send_response(static_cast<size_t>(kev.data));
    if (hres.get_status() == http_response::RES_FINISH) {
        hres.set_status(http_response::RES_IDLE);
        _handler.ev_update(kev.ident, EVFILT_WRITE, EV_DELETE);
    }
}

void server_manager::operate()
{
    while (true) {
        _handler.ev_catch();
        const std::vector<struct kevent>& eventlist = _handler.get_eventlist();
        for (int i = 0; i < _handler.get_nevents(); ++i) {
            switch (_get_type(eventlist[i])) {
            case EVENT_LISTEN:
                _serv_listen(eventlist[i]);
                break;
            case EVENT_HTTP_REQ:
                _serv_http_request(eventlist[i]);
                break;
            case EVENT_HTTP_RES:
                _serv_http_response(eventlist[i]);
                break;
            case EVENT_CGI_REQ:
                _serv_cgi_request(eventlist[i]);
                break;
            case EVENT_CGI_RES:
                _serv_cgi_response(eventlist[i]);
                break;
            case EVENT_ERROR:
                _serv_error(eventlist[i]);
                break;
            }
        }
        //  request_queue 관련 동작 구현해야 함.
    }
}
