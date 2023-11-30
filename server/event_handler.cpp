#include "event_handler.hpp"
#include "Exception.hpp"
#include "fd_info.hpp"

event_handler::event_handler(const std::vector<fd_info>& fd_infov)
: _getv(GETV_SIZE)
{
    struct kevent new_event;

    if ((this->_kq = kqueue()) == -1)
        throw err_syscall();
    for (size_t fd = 3; fd < fd_infov.size(); ++fd) {
        EV_SET(&new_event, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        this->_setv.push_back(new_event);
    }
}

void event_handler::event_catch()
{
    this->_nevents = kevent(this->_kq, (this->_setv.size() ? &this->_setv[0] : NULL),
        this->_setv.size(), (this->_getv.size() ? &this->_getv[0] : NULL),
        this->_getv.size(), &this->timeout);
    this->_setv.clear();
}

void event_handler::event_update(uintptr_t fd, short filter, u_short flags)
{
    struct kevent new_event;

    EV_SET(&new_event, fd, filter, flags, 0, 0, NULL);
    this->_setv.push_back(new_event);
}
