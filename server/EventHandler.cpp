#include <sys/event.h>
#include "Cycle.hpp"
#include "EventHandler.hpp"

EventHandler::EventHandler()
{}

char EventHandler::_getEventType(const struct kevent& kev)
{
    if (kev.flags & EV_ERROR)
        return EVENT_ERROR;
    switch (kev.filter) {
    case EVFILT_READ:
        switch (_kqueueHandler.getEventType(kev.ident)) {
        case KqueueHandler::SOCKET_LISTEN:
            return EVENT_LISTEN;
        case KqueueHandler::SOCKET_CLIENT:
            return EVENT_HTTP_REQ;
        case KqueueHandler::SOCKET_CGI:
            return EVENT_CGI_RES;
        default:
            return EVENT_ERROR;
        }
    case EVFILT_WRITE:
        switch (_kqueueHandler.getEventType(kev.ident)) {
        case KqueueHandler::SOCKET_CLIENT:
            return EVENT_HTTP_RES;
        case KqueueHandler::SOCKET_CGI:
            return EVENT_CGI_REQ;
        default:
            return EVENT_ERROR;
        }
    default:
        return EVENT_ERROR;
    }
}

void EventHandler::_servListen(const struct kevent& kev)
{
    Cycle* cycle;
    int sockfd;
    struct sockaddr_in sin;
    socklen_t len = sizeof(struct sockaddr_in);

    if ((sockfd = accept(kev.ident, NULL, NULL)) == FAILURE)
        throw ERROR;
    if (getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&sin), &len) == FAILURE)
        throw ERROR;
    cycle = Cycle::newCycle(sin.sin_addr.s_addr, sin.sin_port, sockfd);
    _kqueueHandler.addEvent(sockfd, EVFILT_READ, cycle);
    _kqueueHandler.setEventType(sockfd, KqueueHandler::SOCKET_CLIENT);
}

void EventHandler::_servHttpRequest(struct kevent& kev)
{
    /**
     * recvHttpRequest();
     * 
    */
}

void EventHandler::_servHttpResponse(struct kevent& kev)
{
    /**
     * sendHttpResponse();
     * if eof()라면? (전송이 다 끝난다면)
     *     deleteEvent(); (혹은 disable?)
     *     httpResponseHandler의 status를 IDLE로 바꿈.
     *     if queue가 비어있지 않다면?
     *         queue에서 갖고와서 다시 동작. (EventHandler의 private 멤버함수로 구현할 예정)
     *     else if Cycle이 closed상태라면?
     *         cycle 할당 해제
     *         모두 다 없앤다.
     *         
    */
}

void EventHandler::_servCgiRequest(struct kevent& kev)
{
    Cycle* cycle = kev.udata;
    CgiRequestHandler& cgiRequestHandler = cycle->getCgiRequestHandler();

    cgiRequestHandler.sendCgiRequest(kev.ident, static_cast<size_t>(kev.data));
    if (cgiRequestHandler.eof()) {
        close(kev.ident); // -> 자동으로 event는 해제되기 때문에 따로 해제할 필요가 없다.
        _kqueueHandler.deleteEventType(kev.ident);
    }
}

void EventHandler::_servCgiResponse(struct kevent& kev)
{
    Cycle* cycle = kev.udata;
    CgiResponseHandler& cgiResponseHandler = cycle->getCgiResponseHandler();
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();

    cgiResponseHandler.recvCgiResponse(kev.ident, static_cast<size_t>(kev.data));
    if (cgiResponseHandler.eof()) {
        close(kev.ident); // -> 자동으로 event는 해제되기 때문에 따로 해제할 필요가 없다.
        _kqueueHandler.deleteEventType(kev.ident);
        cgiResponseHandler.makeCgiResponse();
        // httpResponseHandler.makeHttpResponse(cgiResponseHandler.getCgiResponse());
        _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
    }
}

void EventHandler::_servError(const struct kevent& kev)
{

}

void EventHandler::initEvent(const std::vector<int>& listenFds)
{
    for (size_t i = 0; i < listenFds.size(); ++i) {
        _kqueueHandler.addEvent(listenFds[i], EVFILT_READ);
        _kqueueHandler.setEventType(listenFds[i], SOCKET_LISTEN);
    }
}

void EventHandler::operate()
{
    struct kevent* eventList = _kqueueHandler.getEventList();

    while (true) {
        _kqueueHandler.eventCatch();
        for (int i = 0; i < _kqueueHandler.getNevents(); ++i) {
            switch (_getEventType(eventList[i])) {
            case EVENT_LISTEN:
                _servListen();
                break;
            case EVENT_HTTP_REQ:
                _servHttpRequest();
                break;
            case EVENT_HTTP_RES:
                _servHttpResponse();
                break;
            case EVENT_CGI_REQ:
                _servCgiRequest();
                break;
            case EVENT_CGI_RES:
                _servCgiResponse();
                break;
            default:
                _servError();
                break;
            }
        }
    }
}
