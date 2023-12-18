#include "webserv.hpp"
#include "event_handler.hpp"
#include "Exception.hpp"

event_handler::event_handler()
: _eventlist(EVENTLIST_SIZE)
{
    if ((_kq = kqueue()) == FAILURE)
        throw err_syscall();
    _timeout.tv_nsec = 0;
    _timeout.tv_sec = 0;
}

void event_handler::ev_catch()
{
    if ((_nevents = kevent(_kq, (_changelist.size() == 0) ? NULL : &_changelist[0],
            _changelist.size(), (_eventlist.size() == 0) ? NULL : &_eventlist[0],
            _eventlist.size(), &_timeout)) == FAILURE)
        throw err_syscall();
    _changelist.clear();
}

void event_handler::ev_update(int ident, short filter, u_short flags)
{
    _changelist.resize(_changelist.size() + 1);
    EV_SET(_changelist.rbegin(), ident, filter, flags, 0, 0, NULL);
}
