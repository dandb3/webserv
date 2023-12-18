#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include <vector>
# include <sys/event.h>
# include <sys/time.h>

# define EVENTLIST_SIZE 10

class event_handler
{
private:
    std::vector<struct kevent> _eventlist;
    std::vector<struct kevent> _changelist;
    struct timespec _timeout;
    int _nevents;
    int _kq;

public:
    event_handler();

    inline int get_nevents();
    inline const std::vector<struct kevent>& get_eventlist();

    void ev_catch();
    void ev_update(int ident, short filter, u_short flags);

};

inline int event_handler::get_nevents()
{
    return _nevents;
}

inline const std::vector<struct kevent>& event_handler::get_eventlist()
{
    return _eventlist;
}

#endif
