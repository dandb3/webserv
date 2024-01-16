#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <vector>
// #include "../webserv.hpp"
// #include "cycle/Cycle.hpp"
#include "KqueueHandler.hpp"

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
        EVENT_SIGCHLD,
        EVENT_ERROR
    };

    KqueueHandler _kqueueHandler;

    char _getEventType(const struct kevent &kev);
    // void _processHttpRequest(Cycle* cycle);

    // void _servListen(const struct kevent& kev);
    // void _servHttpRequest(const struct kevent& kev);
    // void _servHttpResponse(const struct kevent& kev);
    // void _servCgiRequest(const struct kevent& kev);
    // void _servCgiResponse(const struct kevent& kev);
    void _servSigchld(const struct kevent &kev);
    void _servError(const struct kevent &kev);

public:
    EventHandler();

    void initEvent(const std::vector<int> &listenFds);
    KqueueHandler &getKqueueHandler();
    // void operate();
};

#endif
