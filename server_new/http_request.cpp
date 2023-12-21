#include <unistd.h>
#include "http_request.hpp"

http_request::parser::parser(int fd, std::string& r, std::multimap<std::string, std::string>& h, std::string& m)
: _fd(fd), _status(INPUT_REQUEST_LINE), _request_line(r), _header_fields(h), _message_body(m)
{}

void http_request::parser::_input_request_line()
{
    size_t start, end;

    start = 0;
    if ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _http_request_v.push_back(_remain.substr(start, end));
        _remain = _remain.substr(end + 2);
        _status = PARSE_REQUEST_LINE;
    }
}

void http_request::parser::_parse_request_line()
{
    _status = INPUT_HEADER_FIELD;
}

void http_request::parser::_input_header_field()
{
    size_t start, end;
    bool crlf_found = false;

    start = 0;
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        crlf_found = true;
        _http_request_v.push_back(_remain.substr(start, end));
        start = end + 2;
        // if CRLF CRLF comes:
        if (_http_request_v.rbegin()->empty()) {
            _status = PARSE_HEADER_FIELD;
            break;
        }
    }
    if (crlf_found)
        _remain = _remain.substr(end + 2);
}

void http_request::parser::_parse_header_field()
{
    _status = INPUT_MESSAGE_BODY;
}

void http_request::parser::_input_message_body()
{

}

void http_request::parser::input_parse()
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
