#include <sys/event.h>
#include <sys/wait.h>
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
    case EVFILT_SIGNAL:
        return EVENT_SIGCHLD;
    default:
        return EVENT_ERROR;
    }
}

void EventHandler::_processHttpRequest(Cycle* cycle)
{
    HttpResponseHandler& hrspHandler = cycle->getCgiResponseHandler();
    CgiRequestHandler& creqHandler = cycle->getCgiRequestHandler();
    std::queue<HttpRequest>& hreqQueue = cycle->getHttpRequestQueue();

    hrspHandler.setStatus(RES_BUSY);
    hreqHandler.setHttpRequest(hreqQueue.front());
    hreqQueue.pop();
    /**
     * Server block, Location block 선택
     * NetConfig 객체 생성
     * NetConfig, HttpRequest를 통해 CgiRequest를 만드는 것인지, HttpResponse를 만드는 것인지 선택
     * - CgiRequest인 경우
     *   creqHandler.makeCgiRequest(hreqHandler.getHttpRequest());
     *   creqHandler.callCgiScript();
     *   _kqueueHandler.addEvent(cycle->getCgiSendfd(), EVFILT_WRITE, cycle);
     *   _kqueueHandler.setEventType(cycle->getCgiSendfd(), KqueueHandler::SOCKET_CGI);
     * - HttpResponse인 경우
     *   hrspHandler.makeHttpResponse(hreqHandler.getHttpRequest());
     *   _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
    */
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
    Cycle* cycle = kev.udata;
    HttpRequestHandler& hreqHandler = cycle->getHttpRequestHandler();

    /**
     * hreqHandler.recvHttpRequest(kev.ident, static_cast<size_t>(kev.data));
     * hreqHandler.parseHttpRequest(kev.flags & EV_EOF, cycle->getHttpRequestQueue());
     * if (queue가 비어있지 않으면서 httpresponsehandler가 IDLE 상태라면)
     *     _processHttpRequest(cycle);
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
     *         _processHttpRequest(cycle);
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
        _kqueueHandler.addEvent(cycle->getCgiRecvfd(), EVFILT_READ, cycle);
        _kqueueHandler.setEventType(cycle->getCgiRecvfd(), KqueueHandler::SOCKET_CGI);
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

/**
 * kevent에 의하면 kev.data 개수만큼 있는데 0이 리턴되는 경우도 말이 안 된다.
 * 라고 생각했었는데,
 * SIGCHLD가 발생하는 경우를 찾아보니, 자식 프로세스가 종료되는 경우 뿐만 아니라
 * stopped 되거나 continue 되거나 되게 다양한 상황이 존재한다.
 * 그러므로 waitpid() 호출 후 0이 리턴되는 경우도 존재할 수 있다.
 * 다만, SIGCHLD 발생 시 마다 waitpid()를 호출하는 경우가 실제 자식프로세스의
 * 종료되는 횟수보다 많거나 같다는 점은 알고 있어야 한다.
 * (완전히 최적화되지는 않는다는 뜻)
*/

void EventHandler::_servSigchld(const struct kevent& kev)
{
    for (intptr_t i = 0; i < kev.data; ++i)
        if (waitpid(-1, NULL, WNOHANG) == FAILURE)
            throw ERROR;
}

void EventHandler::_servError(const struct kevent& kev)
{

}

void EventHandler::initEvent(const std::vector<int>& listenFds)
{
    _kqueueHandler.addEvent(SIGCHLD, EVFILT_SIGNAL);
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
                _servListen(eventList[i]);
                break;
            case EVENT_HTTP_REQ:
                _servHttpRequest(eventList[i]);
                break;
            case EVENT_HTTP_RES:
                _servHttpResponse(eventList[i]);
                break;
            case EVENT_CGI_REQ:
                _servCgiRequest(eventList[i]);
                break;
            case EVENT_CGI_RES:
                _servCgiResponse(eventList[i]);
                break;
            case EVENT_SIGCHLD:
                _servSigchld(eventList[i]);
            default:
                _servError(eventList[i]);
                break;
            }
        }
    }
}
