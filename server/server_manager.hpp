#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <vector>
# include "event_handler.hpp"

class server_manager
{
private:
    config _conf;
    event_handler _handler;
    std::vector<fd_info> fd_infos;

    void sock_listen();
    void http_request();
    void http_response();
    void cgi_request();
    void cgi_response();
    void serv_error();

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
    }

    void serv_start();

};

#endif
