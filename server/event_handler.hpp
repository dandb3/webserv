#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include <vector>
# include <sys/event.h>

class event_handler
{
private:
    std::vector<struct kevent> _getv;
    std::vector<struct kevent> _setv;
    int _nevents;
    int _kq;

public:
    void event_catch();
    int get_nevents() const;

};

#endif
