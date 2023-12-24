#include <unistd.h>
#include "http_request.hpp"

http_request::http_request(int fd)
    : _fd(fd), _status(INPUT_REQUEST_LINE)
{}

bool http_request::_input_request_line()
{
    size_t start, end;

    start = 0;
    if ((end = _remain.find(CRLF, start)) != std::string::npos) {
        _line_v.push_back(_remain.substr(start, end));
        _remain = _remain.substr(end + 2);
        _status = PARSE_REQUEST_LINE;

        return SUCCESS;
    }
    return FAILURE;
}

bool http_request::_parse_request_line()
{
    std::string request_line = _line_v[0];

    std::istringstream iss(request_line);
    std::vector<std::string> tokens;

    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() != 3)
        return FAILURE;

    // set method
    token = tokens[0];
    if (token == "GET")
        _request_line.set_method(GET);
    else if (token == "HEAD")
        _request_line.set_method(HEAD);
    else if (token == "POST")
        _request_line.set_method(POST);
    else if (token == "DELETE")
        _request_line.set_method(DELETE);
    else
        return FAILURE;

    // set uri
    _request_line.set_uri(tokens[1]);

    // set HTTP version
    token = tokens[2];
    if (token.substr(0, 5) != "HTTP/" || token.length() != 8)
        return FAILURE;

    _request_line.set_version(make_pair(atoi(token[5]), atoi(token[7])));
    return SUCCESS;
}

void http_request::_input_header_field()
{
    size_t start, end;
    bool crlf_found = false;

    start = 0;
    _line_v.clear();
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

bool http_request::_parse_header_field()
{
    size_t pos;
    std::string key, value;

    for (int i = 0; i < _line_v.size(); i++) {
        if ((pos = _line_v[i].find(':', 0)) == std::string::npos)
            return FAILURE;
        cout << "HI\n";
        key = _line_v[i].substr(0, pos);
        value = _line_v[i].substr(pos + 2, _line_v[i].length() - pos - 2);
        _header_fields.insert(make_pair(key, value));
    }
}

void http_request::_input_message_body()
{
    _message_body = _remain;
    if (_header_fields.find("Content-Length") != _header_fields.end())
        _input_default_body();
    else
        _input_chunked_body();
}

void http_request::_input_default_body()
{

}

void http_request::_input_chunked_body()
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
