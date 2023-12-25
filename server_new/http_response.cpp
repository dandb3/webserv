#include <algorithm>
#include <unistd.h>
#include "http_response.hpp"

http_response::http_response(int fd)
: _fd(fd), _pos(0)
{}

void http_response::send_response(size_t size)
{
    size_t write_len;

    write_len = std::min(_response.size() - _pos, size);
    if (write(_fd, _response.c_str() + _pos, write_len) == FAILURE)
        throw err_syscall();
    _pos += write_len;
}
