#include "event_handler.hpp"
#include "Exception.hpp"

event_handler::event_handler()
: _get_v(GETV_SIZE)
{
    if ((_kq = kqueue()) == -1)
        throw err_syscall();
    _timeout.tv_nsec = 0;
    _timeout.tv_sec = 0;
}

void event_handler::event_catch()
{
    _nevents = kevent(_kq, (_set_v.size() ? &_set_v[0] : NULL),
        _set_v.size(), (_get_v.size() ? &_get_v[0] : NULL),
        _get_v.size(), &_timeout);
    _set_v.clear();
}

void event_handler::event_update(uintptr_t fd, short filter, u_short flags)
{
    struct kevent new_event;

    EV_SET(&new_event, fd, filter, flags, 0, 0, NULL);
    _set_v.push_back(new_event);
}
