#include "HttpRequestModule.hpp"

void RequestLine::setMethod(short method)
{
    _method = method;
}

void RequestLine::setUri(std::string uri)
{
    _uri = uri;
}

void RequestLine::setQuery(std::vector<std::pair<std::string, std::string> > &query)
{
    _query = query;
}

void RequestLine::setFragment(std::string fragment)
{
    _fragment = fragment;
}

void RequestLine::setVersion(std::pair<short, short> version)
{
    _version = version;
}

short RequestLine::getMethod() const
{
    return _method;
}

const std::string &RequestLine::getUri() const
{
    return _uri;
}

const std::vector<std::pair<std::string, std::string> > &RequestLine::getQuery() const
{
    return _query;
}

const std::string &RequestLine::getFragment() const
{
    return _fragment;
}

const std::pair<short, short> &RequestLine::getVersion() const
{
    return _version;
}
