#include "HttpRequestModule.hpp"

RequestLine& RequestLine::operator=(const RequestLine& ref)
{
    if (this != ref) {
        _method = ref._method;
        _requestTarget = ref._requestTargetl
        _version = ref._version;
    }
    return *this;
}

void RequestLine::setMethod(short method)
{
    _method = method;
}

void RequestLine::setRequestTarget(std::string requestTarget)
{
    _request_target = requestTarget;
}

void RequestLine::setVersion(std::pair<short, short> version)
{
    _version = version;
}