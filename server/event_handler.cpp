#include "event_handler.hpp"
#include "Exception.hpp"

event_handler::event_handler()
: _getv(GETV_SIZE), _typev(3, SERV_DEFAULT)
{
    if ((_kq = kqueue()) == -1)
        throw err_syscall();
    _timeout.tv_nsec = 0;
    _timeout.tv_sec = 0;
}

void event_handler::event_catch()
{
    _nevents = kevent(_kq, (_setv.size() ? &_setv[0] : NULL),
        _setv.size(), (_getv.size() ? &_getv[0] : NULL),
        _getv.size(), &_timeout);
    _setv.clear();
}

void event_handler::event_update(uintptr_t fd, short filter, u_short flags)
{
    struct kevent new_event;

    EV_SET(&new_event, fd, filter, flags, 0, 0, NULL);
    _setv.push_back(new_event);
}
