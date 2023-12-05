#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <vector>
# include "event_handler.hpp"

class server_manager
{
private:
    config _conf;
    std::vector<fd_info> _fd_infov;
    event_handler _handler;

    void _add_elt(int fd, int type, http* http);
    void _sock_listen();
    void _http_request();
    void _http_response();
    void _cgi_request();
    void _cgi_response();
    void _serv_error();

public:
    enum
    {
        SERV_LISTEN,
        SERV_HTTP_REQ,
        SERV_HTTP_RES,
        SERV_CGI_REQ,
        SERV_CGI_RES,
        SERV_DEFAULT,
        SERV_ERROR
    };

	server_manager(const char* path);

    void serv_start();

};

#endif
