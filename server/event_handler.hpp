#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include <vector>
# include <time.h>
# include <sys/event.h>
# include "fd_info.hpp"

# define GETV_SIZE 100

class event_handler
{
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

private:
    std::vector<struct kevent> _getv;
    std::vector<struct kevent> _setv;
    std::vector<char> _typev;
    struct timespec _timeout;
    int _nevents;
    int _kq;

public:
    event_handler();

    void event_catch();
    void event_update(uintptr_t fd, short filter, u_short flags);
    void add_type(int sockfd, char type);

    inline int get_nevents() const;
    inline bool event_err(int idx) const;

};

inline int event_handler::get_nevents() const
{
    return _nevents;
}

inline bool event_handler::event_err(int idx) const
{
    return (_getv.at(idx).flags & EV_ERROR);
}

#endif
