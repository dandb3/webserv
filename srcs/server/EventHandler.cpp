#include <cstring>
#include <sys/event.h>
#include <sys/wait.h>
#include <signal.h>
#include "EventHandler.hpp"

EventHandler::EventHandler()
{}

char EventHandler::_getEventType(const struct kevent &kev)
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
    case EVFILT_PROC:
        return EVENT_CGI_PROC;
    case EVFILT_TIMER:
        Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);

        if (kev.ident == cycle->getHttpSockfd()) {
            if (cycle->getTimerType() == Cycle::TIMER_REQUEST)
                return EVENT_RTIMER;
            else
                return EVENT_KTIMER;
        }
        else
            return EVENT_CTIMER;
    default:
        return EVENT_ERROR;
    }
}

void EventHandler::_setHttpRequestFromQ(Cycle* cycle)
{
    HttpResponseHandler& hrspHandler = cycle->getHttpResponseHandler();
    HttpRequestHandler& hreqHandler = cycle->getHttpRequestHandler();
    std::queue<HttpRequest>& hreqQ = cycle->getHttpRequestQueue();

    hrspHandler.setStatus(HttpResponseHandler::RES_BUSY);
    hreqHandler.setHttpRequest(hreqQ.front());
    hreqQ.pop();
}

void EventHandler::_processHttpRequest(Cycle* cycle)
{
    ConfigInfo& configInfo = cycle->getConfigInfo();
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    const HttpRequest& httpRequest = cycle->getHttpRequestHandler().getHttpRequest();

    configInfo = ConfigInfo(cycle->getLocalIp(), cycle->getLocalPort(), httpRequest.getUri()); // 얘도 수정 필요. getURI() 함수..
    switch (configInfo.requestType()) {
    case ConfigInfo::CGI_REQUEST:
        CgiRequestHandler& creqHdlr = cycle->getCgiRequestHandler();

        creqHdlr.makeCgiRequest(cycle, httpRequest);
        try {
            creqHdlr.callCgiScript(cycle);
        }
        catch (unsigned short code) {
            httpResponseHandler.makeHttpResponse(HttpRequest(code));
            _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
            break;
        }
        _kqueueHandler.addEvent(cycle->getCgiSendfd(), EVFILT_WRITE, cycle);
        _kqueueHandler.setEventType(cycle->getCgiSendfd(), KqueueHandler::SOCKET_CGI);
        _kqueueHandler.addEvent(cycle->getCgiRecvfd(), EVFILT_READ, cycle);
        _kqueueHandler.setEventType(cycle->getCgiRecvfd(), KqueueHandler::SOCKET_CGI);
        _kqueueHandler.changeEvent(cycle->getCgiScriptPid(), EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT);
        _kqueueHandler.changeEvent(cyle->getCgiSendfd(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER_PERIOD, cycle);
        break;
    case ConfigInfo::HTTP_RESPONSE:
        HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();

        httpResponseHandler.makeHttpResponse(); // 수정 필요. 인자 들어가는거 맞춰서.
        _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
        break;
    }
}

void EventHandler::_servListen(const struct kevent &kev)
{
    Cycle *cycle;
    int sockfd;
    struct sockaddr_in localSin, remoteSin;
    socklen_t localLen = sizeof(struct sockaddr_in);
    socklen_t remoteLen = sizeof(struct sockaddr_in);

    memset(&localSin, 0, localLen);
    memset(&remoteSin, 0, remoteLen);
    if ((sockfd = accept(kev.ident, reinterpret_cast<struct sockaddr*>(&remoteSin), &remoteLen)) == FAILURE)
        return;
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    getsockname(sockfd, reinterpret_cast<struct sockaddr *>(&localSin), &localLen);
    cycle = Cycle::newCycle(localSin.sin_addr.s_addr, localSin.sin_port, remoteSin.sin_addr.s_addr, sockfd);
    _kqueueHandler.addEvent(sockfd, EVFILT_READ, cycle);
    _kqueueHandler.setEventType(sockfd, KqueueHandler::SOCKET_CLIENT);
    _kqueueHandler.changeEvent(sockfd, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER_PERIOD, cycle);
}

/* httpSockFd에 대한 event, eventType, Timer는 설정된 상태 */
void EventHandler::_servHttpRequest(const struct kevent& kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpRequestHandler& httpRequestHandler = cycle->getHttpRequestHandler();
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    std::queue<HttpRequest>& httpRequestQueue = cycle->getHttpRequestQueue();

    httpRequestHandler.recvHttpRequest(kev.ident, static_cast<size_t>(kev.data));
    httpRequestHandler.parseHttpRequest((kev.flags & EV_EOF) && kev.data == 0, cycle->getHttpRequestQueue());
    _kqueueHandler.changeEvent(kev.ident, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER_PERIOD, cycle);
    if (httpRequestHandler.isInputReady())
        cycle->setTimerType(Cycle::TIMER_KEEP_ALIVE);
    else
        cycle->setTimerType(Cycle::TIMER_REQUEST);
    if (httpRequestHandler.closed()) {
        cycle->setClosed();
        _kqueueHandler.deleteEvent(kev.ident, kev.filter);
        _kqueueHandler.deleteEvent(kev.ident, EVFILT_TIMER);
    }
    if (!httpRequestQueue.empty() && httpResponseHandler.getStatus() == HttpResponseHandler::RES_IDLE) {
        _setHttpRequestFromQ(cycle);
        _processHttpRequest(cycle);
    }
}

void EventHandler::_servHttpResponse(const struct kevent& kev)
{
    /**
     * sendHttpResponse();
     * if eof()라면? (전송이 다 끝난다면)


     *     cycle 초기화 (httpResponseHandler의 status를 IDLE로 바꿈. 도 포함)
     *     deleteEvent(); (혹은 disable?)

     *     if queue가 비어있지 않다면?
     *         _processHttpRequest(cycle);
     *     else if Cycle이 closed상태라면?
     *         cycle 할당 해제
     *         모두 다 없앤다.
     *         
    */
}

void EventHandler::_servCgiRequest(const struct kevent& kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    CgiRequestHandler& cgiRequestHandler = cycle->getCgiRequestHandler();

    try {
        cgiRequestHandler.sendCgiRequest(kev);
    }
    catch (unsigned short code) {
        if (cycle->getCgiSendfd() != -1) {
            if (cycle->getCgiScriptPid() != -1)
                kill(cycle->getCgiScriptPid(), SIGKILL);
            if (cycle->getCgiRecvfd() != -1) {
                _kqueueHandler.deleteEventType(cycle->getCgiRecvfd());
                close(cycle->getCgiRecvfd());
                cycle->setCgiRecvfd(-1);
            }
            _kqueueHandler.deleteEventType(cycle->getCgiSendfd());
            close(cycle->getCgiSendfd());
            cycle->setCgiSendfd(-1);
            httpResponseHandler.makeHttpResponse(HttpRequest(code));
            _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
        }
    }
    if (cgiRequestHandler.eof()) {
        close(kev.ident); // -> 자동으로 event는 해제되기 때문에 따로 해제할 필요가 없다.
        cycle->setCgiSendfd(-1);
        _kqueueHandler.deleteEventType(kev.ident);
    }
}

void EventHandler::_servCgiResponse(const struct kevent& kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpRequestHandler& httpRequestHandler = cycle->getHttpRequestHandler();
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    CgiResponseHandler& cgiResponseHandler = cycle->getCgiResponseHandler();

    _kqueueHandler.changeEvent(cycle->getCgiSendfd(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER_PERIOD, cycle);
    try {
        cgiResponseHandler.recvCgiResponse(kev);
    }
    catch (unsigned short code) {
        if (cycle->getCgiRecvfd() != -1) {
            if (cycle->getCgiScriptPid() != -1)
                kill(cycle->getCgiScriptPid(), SIGKILL);
            if (cycle->getCgiSendfd() != -1) {
                _kqueueHandler.deleteEventType(cycle->getCgiSendfd());
                close(cycle->getCgiSendfd());
                cycle->setCgiSendfd(-1);
            }
            _kqueueHandler.deleteEventType(cycle->getCgiRecvfd());
            close(cycle->getCgiRecvfd());
            cycle->setCgiRecvfd(-1);
            httpResponseHandler.makeHttpResponse(HttpRequest(code));
            _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
        }
    }
    if (cgiResponseHandler.eof()) {
        close(kev.ident); // -> 자동으로 event는 제거되기 때문에 따로 제거할 필요가 없다.
        _kqueueHandler.deleteEventType(kev.ident);
        _kqueueHandler.deleteEvent(cycle->getCgiSendfd(), EVFILT_TIMER);
        cgiResponseHandler.makeCgiResponse();
        switch (cgiResponseHandler.getResponseType()) {
        case CgiResponse::LOCAL_REDIR_RES:
            httpRequestHandler.setURI(); // 구현해야 함.
            _processHttpRequest(cycle);
            break;
        case CgiResponse::DOCUMENT_RES:
        case CgiResponse::CLIENT_REDIR_RES:
        case CgiResponse::CLIENT_REDIRDOC_RES:
            _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
            httpResponseHandler.makeHttpResponse(cgiResponseHandler.getCgiResponse());
            break;
        default:    /* in case of an error */
            break;
        }
    }
}

void EventHandler::_servRTimer(const struct kevent &kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    std::queue<HttpRequest>& httpRequestQ = cycle->getHttpRequestQueue();

    _kqueueHandler.deleteEvent(kev.ident, kev.filter);
    _kqueueHandler.deleteEvent(kev.ident, EVFILT_READ);
    cycle->setClosed();
    httpRequestQ.push(HttpRequest(408));
    if (httpResponseHandler.getStatus() == HttpResponseHandler::RES_IDLE) {
        _setHttpRequestFromQ(cycle);
        _processHttpRequest(cycle);
    }
}

void EventHandler::_servKTimer(const struct kevent &kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    std::queue<HttpRequest>& httpRequestQ = cycle->getHttpRequestQueue();

    _kqueueHandler.deleteEvent(kev.ident, kev.filter);
    _kqueueHandler.deleteEvent(kev.ident, EVFILT_READ);
    cycle->setClosed();
    httpRequestQ.push(HttpRequest(408));
    if (httpResponseHandler.getStatus() == HttpResponseHandler::RES_IDLE) {
        _setHttpRequestFromQ(cycle);
        _processHttpRequest(cycle);
    }
}

void EventHandler::_servCTimer(const struct kevent &kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    CgiResponseHandler& cgiResponseHandler = cycle->getCgiResponseHandler();

    _kqueueHandler.deleteEvent(kev.ident, kev.filter);
    if (cycle->getCgiSendfd() != -1 || cycle->getCgiRecvfd() != -1) {
        if (cycle->getCgiScriptPid() != -1)
            kill(cycle->getCgiScriptPid(), SIGKILL);
        if (cycle->getCgiSendfd() != -1) {
            _kqueueHandler.deleteEventType(cycle->getCgiSendfd());
            close(cycle->getCgiSendfd());
            cycle->setCgiSendfd(-1);
        }
        if (cycle->getCgiRecvfd() != -1) {
            _kqueueHandler.deleteEventType(cycle->getCgiRecvfd());
            close(cycle->getCgiRecvfd());
            cycle->setCgiRecvfd(-1);
        }
        httpResponseHandler.makeHttpResponse(HttpRequest(504));
        _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
    }
}

void EventHandler::_servCgiProc(const struct kevent &kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);

    waitpid(kev.ident, NULL, WNOHANG);
    _kqueueHandler.deleteEvent(kev.ident, kev.filter);
    cycle->setCgiScriptPid(-1);
}

void EventHandler::_servError(const struct kevent &kev)
{

}

void EventHandler::initEvent(const std::vector<int> &listenFds)
{
    for (size_t i = 0; i < listenFds.size(); ++i) {
        _kqueueHandler.addEvent(listenFds[i], EVFILT_READ);
        _kqueueHandler.setEventType(listenFds[i], KqueueHandler::SOCKET_LISTEN);
    }
}

void EventHandler::operate()
{
    struct kevent *eventList = _kqueueHandler.getEventList();

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
            case EVENT_CGI_PROC:
                _servCgiProc(eventList[i]);
                break;
            default:
                _servError(eventList[i]);
                break;
            }
        }
    }
}
