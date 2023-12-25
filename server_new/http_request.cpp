#include <unistd.h>
#include <algorithm>
#include "http_request.hpp"

http_request::http_request(int fd)
: _fd(fd), _status(INPUT_REQUEST_LINE), _parsed(false)
{}

void http_request::_input_request_line()
{
    size_t end;

    if ((end = _remain.find(CRLF, 0)) != std::string::npos) {
        _line_v.push_back(_remain.substr(0, end));
        _remain = _remain.substr(end + 2);
        _status = PARSE_REQUEST_LINE;
    }
}

void http_request::_parse_request_line()
{
    _line_v.clear();
    _status = INPUT_HEADER_FIELD;
}

void http_request::_input_header_field()
{
    size_t start, end;
    bool crlf_found = false;

    start = 0;
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        crlf_found = true;
        _line_v.push_back(_remain.substr(start, end));
        start = end + 2;
        // if CRLF CRLF comes:
        if (_line_v.rbegin()->empty()) {
            _status = PARSE_HEADER_FIELD;
            break;
        }
    }
    if (crlf_found)
        _remain = _remain.substr(start);
}

void http_request::_parse_header_field()
{
    _line_v.clear();
    _status = INPUT_MESSAGE_BODY;
}

void http_request::_input_message_body()
{
    _parsed = true;
    _status = INPUT_REQUEST_LINE;
}

void http_request::recv_request(intptr_t size)
{
    ssize_t read_len;

    while (size > 0) {
        if ((read_len = read(_fd, _buf, std::min(size, static_cast<intptr_t>(BUF_SIZE)))) == FAILURE)
            throw err_syscall();
        size -= read_len;
        _buf[read_len] = '\0';
        _remain.append(_buf);
    }
}

void http_request::parse_request()
{
    if (_status == INPUT_REQUEST_LINE)
        _input_request_line();
    if (_status == PARSE_REQUEST_LINE)
        _parse_request_line();
    if (_status == INPUT_HEADER_FIELD)
        _input_header_field();
    if (_status == PARSE_HEADER_FIELD)
        _parse_header_field();
    if (_status == INPUT_MESSAGE_BODY)
        _input_message_body();
}
