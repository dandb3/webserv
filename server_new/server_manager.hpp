#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <map>
# include <vector>
# include <sys/socket.h>
# include <sys/event.h>
# include "event_handler.hpp"
# include "type_checker.hpp"
# include "Exception.hpp"

# define GETV_SIZE 10

class server_manager
{
private:
    config _conf;
    type_checker _checker;
    event_handler _handler;

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

#endif
