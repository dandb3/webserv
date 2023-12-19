#include <unistd.h>
#include "http_request.hpp"

http_request::parser(int fd)
: 
{

}

void http_request::parser::_read_line()
{
    size_t start, end, size;

    if ((size = read(_fd, _buf, BUF_SIZE)) == FAILURE)
        throw err_syscall();
    start = 0;
    _buf[size] = '\0';

    _remain.append(_buf);
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _http_request_line_v.push_back(_remain.substr(start, end));
        start = end + 2;
        if (_http_request_line_v.rbegin()->empty())
            _mode = INPUT_BODY;
    }
    _remain = _remain.substr(start);
}

void http_request::parser::_read_body()
{

}

void http_request::parser::read_buf()
{
    switch (_mode) {
    case INPUT_LINE:
        _read_line();
        break;
    case INPUT_BODY:
        _read_body();
        break;
    }
}

void http_request::read_request()
{

}
