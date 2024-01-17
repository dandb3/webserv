#include "HttpRequestModule.hpp"

RequestLine& RequestLine::operator=(const RequestLine& ref)
{
    if (this != ref) {
        _method = ref._method;
        _requestTarget = ref._requestTarget;
        _version = ref._version;
    }
    return *this;
}

void RequestLine::setMethod(short method)
{
    _method = method;
}

void RequestLine::setRequestTarget(std::string &requestTarget)
{
    _requestTarget = requestTarget;
}

void RequestLine::setVersion(std::pair<short, short> version)
{
    _version = version;
}

const short RequestLine::getMethod() const
{
    return _method;
}

const std::string& RequestLine::getRequestTarget() const
{
    return _requestTarget;
}

const std::pair<short, short>& RequestLine::getVersion() const
{
    return _version;
}
