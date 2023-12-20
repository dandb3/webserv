#include <unistd.h>
#include "http_request.hpp"

http_request::parser::parser(int fd)
: _fd(fd), _status(INPUT_REQUEST_LINE)
{}

void http_request::parser::_read_request_line()
{
    size_t start, end;

    start = 0;
    if ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _http_request_v.push_back(_remain.substr(start, end));
        if (_http_request_v.rbegin()->empty())
            throw 400;  //맞나 모르겠음.. 그래도 header field까지는 받아야 하나?
        _remain = _remain.substr(end + 2);
        _status = INPUT_HEADER_FIELD;
    }
}

void http_request::parser::_read_header_field()
{
    size_t start, end;

    start = 0;
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _http_request_v.push_back(_remain.substr(start, end));
        start = end + 2;
        // if CRLF CRLF comes:
        if (_http_request_v.rbegin()->empty()) {
            _status = INPUT_PARSE;
            break;
        }
    }
    _remain = _remain.substr(end + 2);
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
    else if (size == 0) {
        
    }
    _buf[size] = '\0';
    _remain.append(_buf);
    
    if (_status == INPUT_REQUEST_LINE)
        _read_request_line();
    if (_status == INPUT_HEADER_FIELD)
        _read_header_field();
    if (_status == INPUT_PARSE)
        _parse_front_message();
    if (_status == INPUT_MESSAGE_BODY)
        _read_message_body();
}

void http_request::read_request()
{

}
