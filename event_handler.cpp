#include "Exception.hpp"
#include "event_handler.hpp"

event_handler::event_handler()
{}

event_handler::event_handler(int i)
{

}

event_handler::~event_handler()
{
    if (close(this->_kq) == -1)
        throw Err_syscall();
}

event_handler::event_handler(const event_handler&)
{}

event_handler& event_handler::operator=(const event_handler&)
{
    return *this;
}

int event_handler::event_type(int idx) const
{
    const struct kevent& event = this->_get_vec.at(idx);

    if (this->_listen_fds.find(event.ident) != this->_listen_fds.end())
        return SERVER_LISTEN;
    if (event.filter == EVFILT_READ) {
        if (this->_pipe_fds.find(event.ident) != this->_pipe_fds.end())
            return CGI_RESPONSE;
        else
            return HTTP_REQUEST;
    }
    if (event.filter == EVFILT_WRITE) {
        if (this->_pipe_fds.find(event.ident) != this->_pipe_fds.end())
            return CGI_REQUEST;
        else
            return HTTP_RESPONSE;
    }
    return SERVER_ERROR;
}
