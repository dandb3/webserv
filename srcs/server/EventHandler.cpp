#include <cstring>
#include <sys/event.h>
#include <sys/wait.h>
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
    case EVFILT_SIGNAL:
        return EVENT_SIGCHLD;
    default:
        return EVENT_ERROR;
    }
}

void EventHandler::_setHttpRequestFromQ(Cycle* cycle)
{
    HttpResponseHandler& hrspHandler = cycle->getCgiResponseHandler();
    CgiRequestHandler& creqHandler = cycle->getCgiRequestHandler();
    std::queue<HttpRequest>& hreqQ = cycle->getHttpRequestQueue();

    hrspHandler.setStatus(HttpResponseHandler::RES_BUSY);
    hreqHandler.setHttpRequest(hreqQ.front());
    hreqQ.pop();
}

void EventHandler::_processHttpRequest(Cycle* cycle)
{
    ConfigInfo& configInfo = cycle->getConfigInfo();
    HttpRequest& httpRequest = cycle->getHttpRequestHandler().getHttpRequest();

    configInfo = ConfigInfo(cycle->getLocalIp(), cycle->getLocalPort(), cycle);
    switch (configInfo.requestType()) {
    case ConfigInfo::CGI_REQUEST:
        CgiRequestHandler& creqHdlr = cycle->getCgiRequestHandler();

        creqHdlr.makeCgiRequest(cycle, httpRequest);
        creqHdlr.callCgiScript(cycle);
        _kqueueHandler.addEvent(cycle->getCgiSendfd(), EVFILT_WRITE, cycle);
        break;
    case ConfigInfo::HTTP_RESPONSE:
        break;
    }
    /**
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

void EventHandler::_servListen(const struct kevent &kev)
{
    Cycle *cycle;
    int sockfd;
    struct sockaddr_in localSin, remoteSin;
    socklen_t localLen = sizeof(struct sockaddr_in);
    socklen_t remoteLen = sizeof(struct sockaddr_in);

    memset(&localSin, 0, localLen);
    memset(&remoteSin, 0, remoteLen);
    if ((sockfd = accept(kev.ident, &remoteSin, &remoteLen)) == FAILURE)
        throw ERROR;
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == FAILURE)
        throw ERROR;
    if (getsockname(sockfd, reinterpret_cast<struct sockaddr *>(&localSin), &localLen) == FAILURE)
        throw ERROR;
    cycle = Cycle::newCycle(localSin.sin_addr.s_addr, localSin.sin_port, remoteSin.sin_addr.s_addr, sockfd);
    _kqueueHandler.addEvent(sockfd, EVFILT_READ, cycle);
    _kqueueHandler.setEventType(sockfd, KqueueHandler::SOCKET_CLIENT);
}

void EventHandler::_servHttpRequest(const struct kevent& kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpRequestHandler& hreqHandler = cycle->getHttpRequestHandler();

    /**
     * hreqHandler.recvHttpRequest(kev.ident, static_cast<size_t>(kev.data));
     * hreqHandler.parseHttpRequest(kev.flags & EV_EOF, cycle->getHttpRequestQueue());
     * if (queue가 비어있지 않으면서 httpresponsehandler가 IDLE 상태라면)
     *     _processHttpRequest(cycle);
    */
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
    CgiRequestHandler& cgiRequestHandler = cycle->getCgiRequestHandler();

    cgiRequestHandler.sendCgiRequest(kev);
    if (cgiRequestHandler.eof()) {
        close(kev.ident); // -> 자동으로 event는 해제되기 때문에 따로 해제할 필요가 없다.
        _kqueueHandler.deleteEventType(kev.ident);
        _kqueueHandler.addEvent(cycle->getCgiRecvfd(), EVFILT_READ, cycle);
        _kqueueHandler.setEventType(cycle->getCgiRecvfd(), KqueueHandler::SOCKET_CGI);
    }
}

void EventHandler::_servCgiResponse(const struct kevent& kev)
{
    Cycle* cycle = reinterpret_cast<Cycle*>(kev.udata);
    HttpRequestHandler& httpRequestHandler = cycle->getHttpRequestHandler();
    HttpResponseHandler& httpResponseHandler = cycle->getHttpResponseHandler();
    CgiResponseHandler& cgiResponseHandler = cycle->getCgiResponseHandler();

    cgiResponseHandler.recvCgiResponse(kev);
    if (cgiResponseHandler.eof()) {
        close(kev.ident); // -> 자동으로 event는 제거되기 때문에 따로 제거할 필요가 없다.
        _kqueueHandler.deleteEventType(kev.ident);
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

void EventHandler::_servSigchld(const struct kevent &kev)
{
    for (intptr_t i = 0; i < kev.data; ++i)
        if (waitpid(-1, NULL, WNOHANG) == -1)
            throw 1;
}

void EventHandler::_servError(const struct kevent &kev)
{

}

void EventHandler::initEvent(const std::vector<int> &listenFds)
{
    _kqueueHandler.addEvent(SIGCHLD, EVFILT_SIGNAL);
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
            case EVENT_SIGCHLD:
                _servSigchld(eventList[i]);
                break;
            default:
                _servError(eventList[i]);
                break;
            }
        }
    }
}

