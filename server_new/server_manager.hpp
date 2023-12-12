#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <map>
# include <vector>
# include <sys/socket.h>
# include <sys/event.h>
# include "config.hpp"
# include "fd_info.hpp"
# include "Exception.hpp"

# define GETV_SIZE 10

class server_manager
{
private:
    config _conf;
    std::map<int, fd_info> _fd_info_m;
    std::vector<struct kevent> _get_v;
    std::vector<struct kevent> _set_v;
    int _nevents;
    int _kq;

    inline int _event_type(struct kevent& kev);
    void _event_catch();
    void _event_update(uintptr_t fd, short filter, u_short flags);

    void _serv_listen(struct kevent& kev);
    void _serv_http_request(struct kevent& kev);
    void _serv_http_response(struct kevent& kev);
    void _serv_cgi_request(struct kevent& kev);
    void _serv_cgi_response(struct kevent& kev);
    void _serv_error(struct kevent& kev);

public:
    server_manager(const char* path);

    void operate();

};

inline int server_manager::_event_type(struct kevent& kev)
{
    if (kev.flags & EV_ERROR)
        return fd_info::SERV_ERROR;
    else
        return _fd_info_m.at(kev.ident).get_type();
}

#endif
