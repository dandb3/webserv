#include <sstream>
#include "http_request.hpp"

bool http_request::request_line::parse(std::string line)
{
    std::istringstream iss(line);
    std::vector<std::string> tokens;

    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() != 3)
        return false;

    // decide method
    token = tokens[0];
    if (token == "GET")
        _method = GET;
    else if (token == "HEAD")
        _method = HEAD;
    else if (token == "POST")
        _method = POST;
    else if (token == "DELETE")
        _method = DELETE;
    else
        return false;

    // decide uri
    _uri = tokens[1];

    // decide HTTP version
    token = tokens[2];
    if (token.substr(0, 5) != "HTTP/")
        return false;

    _version = token.substr(5, 3);

    return true;
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