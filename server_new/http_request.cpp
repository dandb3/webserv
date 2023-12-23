#include <unistd.h>
#include "http_request.hpp"

http_request::http_request(int fd)
: _fd(fd), _status(INPUT_REQUEST_LINE)
{}

void http_request::_input_request_line()
{
    size_t start, end;

    start = 0;
    if ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _line_v.push_back(_remain.substr(start, end));
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
        _remain = _remain.substr(end + 2);
}

void http_request::_parse_header_field()
{
    _line_v.clear();
    _status = INPUT_MESSAGE_BODY;
}

void http_request::_input_message_body()
{

}

void http_request::read_input()
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
