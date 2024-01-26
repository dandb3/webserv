#include "HttpRequestQueue.hpp"

HttpRequestQueue::HttpRequestQueue()
: _errorCode(0)
{}

void HttpRequestQueue::push(const HttpRequest& httpRequest)
{
    _q.push(httpRequest);
}

void HttpRequestQueue::pop()
{
    _q.pop();
}

const HttpRequest& HttpRequestQueue::front() const
{
    return _q.front();
}

bool HttpRequestQueue::empty() const
{
    return _q.empty();
}

void HttpRequestQueue::setErrorStatus(unsigned short errorCode, const std::string& reasonPhrase)
{
    _errorCode = errorCode;
    _reasonPhrase = reasonPhrase;
}

unsigned short HttpRequestQueue::getErrorCode() const
{
    return _errorCode;
}
