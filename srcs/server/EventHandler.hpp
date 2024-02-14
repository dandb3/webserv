#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <vector>
#include "../webserv.hpp"
#include "./cycle/Cycle.hpp"
#include "KqueueHandler.hpp"

class Cycle;

class EventHandler
{
private:
    enum EventType_e
    {
        EVENT_LISTEN,
        EVENT_HTTP_REQ,
        EVENT_HTTP_RES,
        EVENT_CGI_REQ,
        EVENT_CGI_RES,
        EVENT_CGI_PROC,
        EVENT_FILE_READ,
        EVENT_FILE_WRITE,
        EVENT_STIMER,
        EVENT_CTIMER,
        EVENT_ERROR
    };

    KqueueHandler _kqueueHandler;

    char _getEventType(const struct kevent &kev);
    void _setHttpResponseEvent(Cycle* cycle);
    void _setHttpRequestFromQ(Cycle* cycle);
    void _processHttpRequest(Cycle* cycle);
    void _checkClientBodySize(Cycle* cycle);

    void _servListen(const struct kevent& kev);
    void _servHttpRequest(const struct kevent& kev);
    void _servHttpResponse(const struct kevent& kev);
    void _servCgiRequest(const struct kevent& kev);
    void _servCgiResponse(const struct kevent& kev);
    void _servCgiProc(const struct kevent& kev);
    void _servFileRead(const struct kevent& kev);
    void _servFileWrite(const struct kevent& kev);
    void _servSTimer(const struct kevent& kev);
    void _servCTimer(const struct kevent& kev);

public:
    EventHandler();

    void initEvent(const std::vector<int> &listenFds);
    void operate();
};

#endif
