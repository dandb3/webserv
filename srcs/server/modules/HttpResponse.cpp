#include <algorithm>
#include <unistd.h>
#include "HttpResponseModule.hpp"

HttpResponse::HttpResponse(int fd)
: _fd(fd), _status(RES_IDLE), _pos(0)
{

}

void HttpResponse::send_response(size_t size)
{
    size_t write_len;

    write_len = std::min(_response.size() - _pos, size);
    if (write(_fd, _response.c_str() + _pos, write_len) == FAILURE)
        throw err_syscall();
    _pos += write_len;
    if (_pos == size) {
        _status = RES_IDLE;
        _pos = 0;
    }
}

int HttpResponse::get_status() const
{
	return _status;
}

void HttpResponse::set_status(int status)
{
	_status = static_cast<char>(status);
}
