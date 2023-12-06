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
    void _http_request();
    void _http_response();
    void _cgi_request();
    void _cgi_response();
    void _serv_error();

public:
	server_manager(const char* path);

    void serv_start();

};

#endif
