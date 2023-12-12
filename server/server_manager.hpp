#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <map>
# include <vector>
# include "fd_info.hpp"
# include "event_handler.hpp"

class server_manager
{
private:
    config _conf;
    std::map<int, fd_info*> _fd_info_m;
    event_handler _handler;

    void _sock_listen();
    void _serv_http_request();
    void _serv_http_response();
    void _serv_cgi_request();
    void _serv_cgi_response();
    void _serv_error();

public:
	server_manager(const char* path);

    void serv_start();

};

#endif
