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
