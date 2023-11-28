#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <vector>
# include "event_handler.hpp"

class server_manager
{
private:
    enum
    {
        SOCK_LISTEN,
        HTTP_REQUEST,
        HTTP_RESPONSE,
        CGI_REQUEST,
        CGI_RESPONSE,
        SERV_ERROR
    }

    event_handler _handler;
    std::vector<fd_info> fd_infos;

    void sock_listen();
    void http_request();
    void http_response();
    void cgi_request();
    void cgi_response();
    void serv_error();

public:
    void serv_start();

};

#endif
