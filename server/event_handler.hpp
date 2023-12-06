#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include <map>
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
    std::vector<struct kevent> _get_v;
    std::vector<struct kevent> _set_v;
    std::map<int, char> _type_m;
    struct timespec _timeout;
    int _nevents;
    int _kq;

public:
    event_handler();

    void event_catch();
    void event_update(uintptr_t fd, short filter, u_short flags);

    inline int get_nevents() const;
    inline char event_type(int idx) const;
    inline void add_type(int sockfd, char type);
    inline void rm_type(int sockfd);

};

inline int event_handler::get_nevents() const
{
    return _nevents;
}

inline void event_handler::add_type(int sockfd, char type)
{
    _type_m.insert(std::make_pair(sockfd, type));
}

inline void event_handler::rm_type(int sockfd)
{
    _type_m.erase(sockfd);
}

#endif
