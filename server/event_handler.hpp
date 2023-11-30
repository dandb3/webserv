#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include <vector>
# include <time.h>
# include <sys/event.h>
# include "config.hpp"

# define GETV_SIZE 100

class event_handler
{
private:
    std::vector<struct kevent> _getv;
    std::vector<struct kevent> _setv;
    struct timespec _timeout;
    int _nevents;
    int _kq;

public:
    event_handler(const std::vector<fd_info>& fd_infov);

    void event_catch();
    void event_update(uintptr_t fd, short filter, u_short flags);

    inline int get_nevents() const;
    inline bool event_err(int idx) const;

};

inline int event_handler::get_nevents() const
{
    return this->_nevents;
}

inline bool event_handler::event_err(int idx) const
{
    return (this->_getv.at(idx).flags & EV_ERROR);
}

#endif
