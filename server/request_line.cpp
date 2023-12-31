#include <sstream>
#include "http_request_parser.hpp"

request_line::request_line(short method, std::string request_target, std::pair<short, short> version) : _method(method), _request_target(request_target), _version(version) {}

request_line &request_line::operator=(const request_line &ref)
{
    _method = ref.get_method();
    _request_target = ref.get_request_target();
    _version = ref.get_version();
    return *this;
}

void http_request_parser::request_line::set_method(short method)
{
    _method = method;
}

void http_request_parser::request_line::set_request_target(std::string _request_target)
{
    _request_target = request_target;
}

void http_request_parser::request_line::set_version(std::pair<short, short> version)
{
    _version = version;
}

int http_request_parser::request_line::get_method() const
{
    return this->_method;
}

std::string http_request_parser::request_line::get_request_target() const
{
    return this->_request_target;
}

std::string http_request_parser::request_line::get_version() const
{
    return this->_version;
}