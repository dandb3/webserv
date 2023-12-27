#include <unistd.h>
#include <algorithm>
#include "http_request_parser.hpp"

http_request_parser::http_request_parser(int fd)
: _fd(fd), _status(INPUT_READY)
{}

void http_request_parser::_input_start()
{
    _status = INPUT_REQUEST_LINE;
}

void http_request_parser::_input_request_line()
{
    size_t end;

    if ((end = _remain.find(CRLF, 0)) != std::string::npos) {
        _line_v.push_back(_remain.substr(0, end));
        _remain = _remain.substr(end + 2);
        _status = PARSE_REQUEST_LINE;
    }
}

void http_request_parser::_parse_request_line()
{
    _line_v.clear();
    _status = INPUT_HEADER_FIELD;
}

void http_request_parser::_input_header_field()
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

void http_request_parser::_parse_header_field()
{
    _line_v.clear();
    _status = INPUT_MESSAGE_BODY;
}

void http_request_parser::_input_message_body()
{
    _parsed = true;
    _status = INPUT_REQUEST_LINE;
}

void http_request_parser::_push_err_request()
{
    _http_request_q.push(http_request(_request_line, _header_field, _message_body));
    _status = INPUT_CLOSED;
}

void http_request_parser::_push_request()
{
    _http_request_q.push(http_request(_request_line, _header_field, _message_body));
    _status = INPUT_READY;
    // 만약 Connection: closed라면 -> _status = INPUT_CLOSED;
}

void http_request_parser::recv_request(size_t size)
{
    ssize_t read_len;

    while (size > 0) {
        if ((read_len = read(_fd, _buf, std::min(size, static_cast<size_t>(BUF_SIZE)))) == FAILURE)
            throw err_syscall();
        size -= read_len;
        _buf[read_len] = '\0';
        _remain.append(_buf);
    }
}

void http_request_parser::parse_request(bool eof)
{
    do {
        // 사실 여기서 status 값으로 PARSE_* 애들은 빼도 될 것 같음.
        // 그냥 _input_*함수 내부로 집어넣어도 전혀 상관 없을듯.
        if (_status == INPUT_READY)
            _input_start();
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
        if (_status != PARSE_FINISHED && eof)
            _push_err_request();
        if (_status == PARSE_FINISHED)
            _push_request();
    } while (_status == INPUT_READY);
}
