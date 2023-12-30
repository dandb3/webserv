#include <unistd.h>
#include "http_request.hpp"

http_request::http_request(int fd)
    : _fd(fd), _status(INPUT_REQUEST_LINE)
{}

bool http_request::_input_request_line()
{
    size_t start, end;

    start = 0;
    if ((end = _remain.find(CRLF, start)) == std::string::npos)
        return FAILURE;
 
    _line_v.push_back(_remain.substr(start, end));
    _remain = _remain.substr(end + 2);
    _status = PARSE_REQUEST_LINE;

    return SUCCESS;
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

    if (tokens.size() != 3) // 400 ERROR
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
    else // 처리하지 않을 header
        return FAILURE;

    // set uri
    _request_line.set_request_target(tokens[1]);

    // set HTTP version
    token = tokens[2];
    if (token.substr(0, 5) != "HTTP/" || token.length() != 8)
        return FAILURE;

    _request_line.set_version(make_pair(atoi(token[5]), atoi(token[7])));
    _status = PARSE_REQUEST_LINE;
    return SUCCESS;
}

void http_request::_input_header_field()
{
    const std::string whitespace = WHITESPACE;
    size_t start, end;
    bool crlf_found = false;

    start = 0;
    _line_v.clear();
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        crlf_found = true;

        _line_v.push_back(_remain.substr(start, end - (whitespace.find(_remain[end - 1]) != string::npos))); // remove last OWS
        start = end + 2;

        // if CRLF CRLF comes:
        if (_line_v.rbegin()->empty()) {
            _status = PARSE_HEADER_FIELD;
            break;
        }
    }
    if (crlf_found)
        _remain = _remain.substr(end + 2);

    _status = PARSE_HEADER_FIELD;
}

bool http_request::_parse_header_field()
{
    const std::string whitespace = WHITESPACE;
    std::string key, value;
    size_t pos;

    for (int i = 0; i < _line_v.size(); i++) {
        if ((pos = _line_v[i].find(':')) == std::string::npos)
            return FAILURE; // maybe a obs-fold -> 400 ERROR
        if (whitespace.find(_line_v[i][pos - 1]) != std::string::npos)
            return FAILURE; // 400 ERROR

        key = _line_v[i].substr(0, pos);
        pos = pos + 1 + (whitespace.find(_line_v[i][pos]) != std::string::npos);
        value = _line_v[i].substr(pos, _line_v[i].length() - pos);
        _header_fields.insert(make_pair(key, value));
    }

    _status = INPUT_MESSAGE_BODY;
    return SUCCESS;
}

/*
Case of the messsage body
1. content-length O, transfer-encoding X -> message body 그대로
2. content-length X, transfer-encoding X -> request message가 payload body를 가지고 있지 않고, method가 body에 의미를 두지 않은 경우
3. content-length X, transfer-encoding: chunked
4. content-length X, transfer-encoding: gzip
5. content-length O, transfer-encoding O -> sender MUST NOT
6. content-length가 여러개 있거나 list형태로 오는 경우
*/
void http_request::_input_message_body()
{
    int count = _header_fields.count("Content-Length");
    if (count > 0)
        _input_default_body(count)
    else
        _input_chunked_body();
}

void http_request::_input_default_body(int count)
{
    std::string length_string;
    long long length;

    if (_header_fields.count("Transfer-Encoding") > 0) // sender MUST NOT
        return;
    if (count == 1)
        length_string = _header_fields["Content-Length"];
    else {
        std::multimap<std::string, std::string>::iterator iter;
        string length_string;
        length_string = std::strtol(iter->second);
        for (iter++; iter != _header_fields.end() && iter->first == "Content-Length"; iter++) {
            if (length_string != iter->second)
                return;
        }
    }
    if (length_string.find(',') != std::string::npos) // content-length가 x, y 형태로 들어온 경우
        return;
    length = std::strtol(length_string);

    _message_body += _remain.substr(0, length);
    _status = INPUT_FINISH;
}

void http_request::_input_chunked_body()
{
    enum {
        LENGTH = 0,
        STRING
    };

    size_t start, end;
    long long length;
    bool mode = LENGTH;

    start = 0;
    _line_v.clear();
    while ((end = _remain.find(CRLF, start)) != std::string::npos) {
        if (mode == LENGTH) {
            length = strtol(_remain.substr(start, end));
            mode = STRING;
        }
        else {
            _message_body += _remain.substr(start, length);
            mode = LENGTH;
        }
        start += 2;
    }
    _status = INPUT_FINISH;
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
