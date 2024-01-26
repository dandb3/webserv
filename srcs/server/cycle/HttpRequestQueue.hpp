#ifndef HTTP_REQUEST_QUEUE
#define HTTP_REQUEST_QUEUE

#include <queue>
#include "../modules/HttpRequestModule.hpp"

class HttpRequestQueue
{
private:
    std::queue<HttpRequest> _q;
    unsigned short _errorCode;
    std::string _reasonPhrase;

public:
    HttpRequestQueue();

    void push(const HttpRequest& httpRequest);
    void pop();
    const HttpRequest& front() const;
    bool empty() const;

    void setErrorStatus(unsigned short errorCode, const std::string& reasonPhrase);
    unsigned short getErrorCode() const;

};

#endif
