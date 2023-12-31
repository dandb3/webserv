#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <map>
# include <vector>
# include <sys/socket.h>
# include <sys/event.h>
# include "event_handler.hpp"
# include "http_request.hpp"
# include "webserv.hpp"

# define GETV_SIZE 10

class server_manager
{
private:
    enum
    {
        SERV_LISTEN,
        SERV_HTTP_REQ,
        SERV_HTTP_RES,
        SERV_CGI_REQ,
        SERV_CGI_RES,
        SERV_ERROR,
    };

    config _conf;
    std::map<int, char> _type_m;
    std::map<int, http_request_parser> _http_request_m;
    event_handler _handler;

//  get type corresponding to fd value
    inline int _get_type(const struct kevent& kev);

//  main functions of this class
    void _serv_listen(const struct kevent& kev);
    void _serv_http_request(const struct kevent& kev);
    void _serv_http_response(const struct kevent& kev);
    void _serv_cgi_request(const struct kevent& kev);
    void _serv_cgi_response(const struct kevent& kev);
    void _serv_error(const struct kevent& kev);

public:
    server_manager(const char* path);

    void operate();

};

inline int server_manager::_get_type(const struct kevent& kev)
{
    if (kev.flags & EV_ERROR)
        return SERV_ERROR;
    else
        return _type_m[kev.ident];
}

#endif
