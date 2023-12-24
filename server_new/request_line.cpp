#include <sstream>
#include "http_request.hpp"

request_line::request_line(short method, std::string uri, std::pair<short, short> version) : _method(method), _uri(uri), _version(version) {}

request_line &request_line::operator=(const request_line &ref)
{
    _method = ref.get_method();
    _uri = ref.get_uri();
    _version = ref.get_version();
    return *this;
}

void http_request::request_line::set_method(short method)
{
    _method = method;
}

void http_request::request_line::set_uri(std::string _uri)
{
    _uri = uri;
}

void http_request::request_line::set_version(std::pair<short, short> version)
{
    _version = version;
}

int http_request::request_line::get_method() const
{
    return this->_method;
}

std::string http_request::request_line::get_uri() const
{
    return this->_uri;
}

std::string http_request::request_line::get_version() const
{
    return this->_version;
}