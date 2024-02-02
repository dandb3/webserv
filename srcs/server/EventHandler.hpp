#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <vector>
#include "../webserv.hpp"
#include "cycle/Cycle.hpp"
#include "KqueueHandler.hpp"

#define TIMER_PERIOD 5

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
        EVENT_FILE,
        EVENT_RTIMER,
        EVENT_KTIMER,
        EVENT_CTIMER,
        EVENT_ERROR
    };

    KqueueHandler _kqueueHandler;

    char _getEventType(const struct kevent &kev);
    void _setHttpRequestFromQ(Cycle* cycle);
    void _processHttpRequest(Cycle* cycle);

    void _servListen(const struct kevent& kev);
    void _servHttpRequest(const struct kevent& kev);
    void _servHttpResponse(const struct kevent& kev);
    void _servCgiRequest(const struct kevent& kev);
    void _servCgiResponse(const struct kevent& kev);
    void _servCgiProc(const struct kevent& kev);
    void _servFile(const struct kevent& kev);
    void _servRTimer(const struct kevent& kev);
    void _servKTimer(const struct kevent& kev);
    void _servCTimer(const struct kevent& kev);
    void _servError(const struct kevent &kev);

public:
    EventHandler();

    void initEvent(const std::vector<int> &listenFds);
    void operate();
};

#endif
