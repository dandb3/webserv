#include <unistd.h>
#include "http_request.hpp"

http_request::parser::parser(int fd)
: _fd(fd), _mode(INPUT_REQUEST_LINE)
{}

void http_request::parser::_read_request_line()
{
    ssize_t size;
    size_t start, end;

    if ((size = read(_fd, _buf, BUF_SIZE)) == FAILURE)
        throw err_syscall();
    _buf[size] = '\0';
    start = _remain.size();
    _remain.append(_buf);

    if ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _http_request_v.push_back(_remain.substr(0, end));
        _remain = _remain.substr(end);
        _mode = INPUT_HEADER_FIELD;
    }
}

void http_request::parser::_read_header_field()
{
    ssize_t size;
    size_t start, end;

    if ((size = read(_fd, _buf, BUF_SIZE)) == FAILURE)
        throw err_syscall();
    _buf[size] = '\0';
    start = 0;
    _remain.append(_buf);

    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _http_request_v.push_back(_remain.substr(start, end));
        start = end + 2;
        // if CRLF
        if (_http_request_v.rbegin()->empty()) {
            _mode = INPUT_MESSAGE_BODY;
            break;
        }
    }
    _remain = _remain.substr(start);
}

void http_request::parser::_read_message_body()
{

}

void http_request::parser::read_buf()
{
    ssize_t size;

    size = read(_fd, _buf, BUF_SIZE);
    if (size == FAILURE)
        throw err_syscall();
    else if (size == 0)
    
    
    if (_mode == INPUT_REQUEST_LINE)
        _read_request_line();
    if (_mode == INPUT_HEADER_FIELD)
        _read_header_field();
    if (_mode == INPUT_MESSAGE_BODY)
        _read_message_body();
}

void http_request::read_request()
{

}
