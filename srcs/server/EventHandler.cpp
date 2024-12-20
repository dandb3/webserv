#include <cstring>
#include <csignal>
#include <sys/event.h>
#include <sys/wait.h>
#include "EventHandler.hpp"
#include "modules/PidSet.hpp"

EventHandler::EventHandler()
{}

char EventHandler::_getEventType(const struct kevent &kev)
{
    Cycle* cycle;

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
        case KqueueHandler::FILE_OPEN:
            return EVENT_FILE_READ;
        default:
            return EVENT_ERROR;
        }
    case EVFILT_WRITE:
        switch (_kqueueHandler.getEventType(kev.ident)) {
        case KqueueHandler::SOCKET_CLIENT:
            return EVENT_HTTP_RES;
        case KqueueHandler::SOCKET_CGI:
            return EVENT_CGI_REQ;
        case KqueueHandler::FILE_OPEN:
            return EVENT_FILE_WRITE;
        default:
            return EVENT_ERROR;
        }
    case EVFILT_PROC:
        return EVENT_CGI_PROC;
    case EVFILT_TIMER:
        cycle = reinterpret_cast<Cycle*>(kev.udata);

        if (static_cast<int>(kev.ident) == cycle->getHttpSockfd())
            return EVENT_STIMER;
        else
            return EVENT_CTIMER;
    default:
        return EVENT_ERROR;
    }
}

void EventHandler::_setHttpResponseEvent(Cycle *cycle)
{
    std::map<int, WriteFile> &writeFiles = cycle->getWriteFiles();
    int readFile = cycle->getReadFile();

    if (readFile != -1) {
        _kqueueHandler.addEvent(readFile, EVFILT_READ, cycle);
        _kqueueHandler.setEventType(readFile, KqueueHandler::FILE_OPEN);
    }
    else if (!writeFiles.empty()) {
        for (std::map<int, WriteFile>::iterator it = writeFiles.begin(); it != writeFiles.end(); ++it) {
            _kqueueHandler.addEvent(it->first, EVFILT_WRITE, cycle);
            _kqueueHandler.setEventType(it->first, KqueueHandler::FILE_OPEN);
        }
    }
    else
        _kqueueHandler.addEvent(cycle->getHttpSockfd(), EVFILT_WRITE, cycle);
}

void EventHandler::_setHttpRequestFromQ(Cycle *cycle)
{
    HttpResponseHandler &hrspHandler = cycle->getHttpResponseHandler();
    HttpRequestHandler &hreqHandler = cycle->getHttpRequestHandler();
    std::queue<HttpRequest> &hreqQ = cycle->getHttpRequestQueue();

    hrspHandler.setStatus(HttpResponseHandler::RES_BUSY);
    hreqHandler.setHttpRequest(hreqQ.front());
    hreqQ.pop();
}

void EventHandler::_checkClientBodySize(Cycle *cycle)
{
    ConfigInfo &configInfo = cycle->getConfigInfo();
    HttpRequest &httpRequest = cycle->getHttpRequestHandler().getHttpRequest();

    if (httpRequest.getCode() != 0)
        return;

    const t_directives &info = configInfo.getInfo();
    t_directives::const_iterator it = info.find("limit_client_body_size");

    const size_t maxBodySize = static_cast<size_t>(strtoul(it->second[0].c_str(), NULL, 10));
    if (httpRequest.getMessageBody().length() > maxBodySize)
        httpRequest.setCode(413);
}

void EventHandler::_processHttpRequest(Cycle *cycle)
{
    ConfigInfo &configInfo = cycle->getConfigInfo();
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();
    HttpRequest &httpRequest = cycle->getHttpRequestHandler().getHttpRequest();

    configInfo = ConfigInfo(cycle->getLocalIp(), cycle->getLocalPort(), \
        httpRequest.getHeaderFields().find("Host")->second, httpRequest.getRequestLine().getUri());

    if (httpRequest.getCode() == 0)
        _checkClientBodySize(cycle);
    if (configInfo.requestType(httpRequest) == ConfigInfo::MAKE_HTTP_RESPONSE) {
        httpResponseHandler.makeHttpResponse(cycle, httpRequest);
        _setHttpResponseEvent(cycle);
    }
    else {
        CgiRequestHandler &creqHdlr = cycle->getCgiRequestHandler();

        creqHdlr.makeCgiRequest(cycle, httpRequest);
        try {
            creqHdlr.callCgiScript(cycle);
        }
        catch (int code) {
            unsigned short ucode = static_cast<unsigned short>(code);
            httpRequest.setCode(ucode);
            httpResponseHandler.makeHttpResponse(cycle, httpRequest);
            _setHttpResponseEvent(cycle);
            return;
        }
        _kqueueHandler.addEvent(cycle->getCgiSendfd(), EVFILT_WRITE, cycle);
        _kqueueHandler.setEventType(cycle->getCgiSendfd(), KqueueHandler::SOCKET_CGI);
        _kqueueHandler.addEvent(cycle->getCgiRecvfd(), EVFILT_READ, cycle);
        _kqueueHandler.setEventType(cycle->getCgiRecvfd(), KqueueHandler::SOCKET_CGI);
        _kqueueHandler.changeEvent(cycle->getCgiScriptPid(), EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT);
        _kqueueHandler.changeEvent(cycle->getCgiRecvfd(), EVFILT_TIMER, EV_ADD, NOTE_SECONDS, DEFAULT_TIMEOUT, cycle);
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
    if ((sockfd = accept(kev.ident, reinterpret_cast<struct sockaddr *>(&remoteSin), &remoteLen)) == FAILURE)
        return;
    fcntl(sockfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
    getsockname(sockfd, reinterpret_cast<struct sockaddr *>(&localSin), &localLen);
    cycle = Cycle::newCycle(localSin.sin_addr.s_addr, localSin.sin_port, remoteSin.sin_addr.s_addr, sockfd);
    _kqueueHandler.addEvent(sockfd, EVFILT_READ, cycle);
    _kqueueHandler.setEventType(sockfd, KqueueHandler::SOCKET_CLIENT);
    _kqueueHandler.changeEvent(sockfd, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, DEFAULT_TIMEOUT, cycle);
}

/* httpSockFd에 대한 event, eventType, Timer는 설정된 상태 */
void EventHandler::_servHttpRequest(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpRequestHandler &httpRequestHandler = cycle->getHttpRequestHandler();
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();
    std::queue<HttpRequest> &httpRequestQueue = cycle->getHttpRequestQueue();

    if (cycle->beDeleted())
        return;

    httpRequestHandler.recvHttpRequest(kev.ident, static_cast<size_t>(kev.data));
    httpRequestHandler.parseHttpRequest((kev.flags & EV_EOF) && kev.data == 0, cycle->getHttpRequestQueue());
    if (httpRequestHandler.isInputReady())
        _kqueueHandler.changeEvent(kev.ident, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, DEFAULT_TIMEOUT, cycle);
    else
        _kqueueHandler.changeEvent(kev.ident, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, DEFAULT_TIMEOUT, cycle);
    if (httpRequestHandler.closed()) {
        cycle->setClosed();
        _kqueueHandler.deleteEvent(kev.ident, EVFILT_READ);
        _kqueueHandler.deleteEvent(kev.ident, EVFILT_TIMER);
        if (httpRequestQueue.empty() && httpResponseHandler.getStatus() == HttpResponseHandler::RES_IDLE)
            cycle->setBeDeleted();
    }
    if (!httpRequestQueue.empty() && httpResponseHandler.getStatus() == HttpResponseHandler::RES_IDLE) {
        _setHttpRequestFromQ(cycle);
        _processHttpRequest(cycle);
    }
}

void EventHandler::_servHttpResponse(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();

    if (cycle->beDeleted())
        return;

    httpResponseHandler.sendHttpResponse(kev.ident, static_cast<size_t>(kev.data));
    if (httpResponseHandler.getStatus() == HttpResponseHandler::RES_FINISH) {
        cycle->reset();
        if (!cycle->getHttpRequestQueue().empty()) {
            _kqueueHandler.deleteEvent(kev.ident, EVFILT_WRITE);
            _setHttpRequestFromQ(cycle);
            _processHttpRequest(cycle);
        }
        else if (cycle->closed())
            cycle->setBeDeleted();
        else
            _kqueueHandler.deleteEvent(kev.ident, EVFILT_WRITE);
    }
}

void EventHandler::_servCgiRequest(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpRequestHandler &httpRequestHandler = cycle->getHttpRequestHandler();
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();
    CgiRequestHandler &cgiRequestHandler = cycle->getCgiRequestHandler();

    if (cycle->beDeleted())
        return;

    try {
        cgiRequestHandler.sendCgiRequest(kev);
    }
    catch (int code) {
        unsigned short ucode = static_cast<unsigned short>(code);
        if (cycle->getCgiSendfd() != -1) {
            if (PidSet::found(cycle->getCgiScriptPid()))
                kill(cycle->getCgiScriptPid(), SIGKILL);
            if (cycle->getCgiRecvfd() != -1) {
                _kqueueHandler.deleteEvent(cycle->getCgiRecvfd(), EVFILT_TIMER);
                _kqueueHandler.deleteEventType(cycle->getCgiRecvfd());
                close(cycle->getCgiRecvfd());
                cycle->setCgiRecvfd(-1);
            }
            _kqueueHandler.deleteEventType(cycle->getCgiSendfd());
            close(cycle->getCgiSendfd());
            cycle->setCgiSendfd(-1);
            httpRequestHandler.getHttpRequest().setCode(ucode);
            httpResponseHandler.makeHttpResponse(cycle, httpRequestHandler.getHttpRequest());
            _setHttpResponseEvent(cycle);
        }
        return;
    }
    if (cgiRequestHandler.eof()) {
        close(kev.ident);
        cycle->setCgiSendfd(-1);
        _kqueueHandler.deleteEventType(kev.ident);
    }
}

void EventHandler::_servCgiResponse(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpRequestHandler &httpRequestHandler = cycle->getHttpRequestHandler();
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();
    CgiResponseHandler &cgiResponseHandler = cycle->getCgiResponseHandler();

    if (cycle->beDeleted())
        return;

    _kqueueHandler.changeEvent(cycle->getCgiRecvfd(), EVFILT_TIMER, EV_ADD, NOTE_SECONDS, DEFAULT_TIMEOUT, cycle);
    try {
        cgiResponseHandler.recvCgiResponse(kev);
    }
    catch (int code) {
        unsigned short ucode = static_cast<unsigned short>(code);
        if (cycle->getCgiRecvfd() != -1) {
            if (PidSet::found(cycle->getCgiScriptPid()))
                kill(cycle->getCgiScriptPid(), SIGKILL);
            if (cycle->getCgiSendfd() != -1) {
                _kqueueHandler.deleteEventType(cycle->getCgiSendfd());
                close(cycle->getCgiSendfd());
                cycle->setCgiSendfd(-1);
            }
            _kqueueHandler.deleteEvent(cycle->getCgiRecvfd(), EVFILT_TIMER);
            _kqueueHandler.deleteEventType(cycle->getCgiRecvfd());
            close(cycle->getCgiRecvfd());
            cycle->setCgiRecvfd(-1);
            httpRequestHandler.getHttpRequest().setCode(ucode);
            httpResponseHandler.makeHttpResponse(cycle, httpRequestHandler.getHttpRequest());
            _setHttpResponseEvent(cycle);
        }
        return;
    }
    if (cgiResponseHandler.eof()) {
        if (PidSet::found(cycle->getCgiScriptPid()))
            kill(cycle->getCgiScriptPid(), SIGKILL);
        close(kev.ident);
        _kqueueHandler.deleteEventType(kev.ident);
        _kqueueHandler.deleteEvent(cycle->getCgiRecvfd(), EVFILT_TIMER);
        cycle->setCgiRecvfd(-1);
        cgiResponseHandler.makeCgiResponse();
        if (httpRequestHandler.getHttpRequest().getRequestLine().getMethod() == HttpRequestHandler::HEAD)
            cgiResponseHandler.getCgiResponse().getMessageBody().clear();
        switch (cgiResponseHandler.getResponseType()) {
        case CgiResponse::LOCAL_REDIR_RES:
            httpRequestHandler.getHttpRequest().getRequestLine().setUri(cgiResponseHandler.getCgiResponse().getHeaderFields().at(0).second);
            _processHttpRequest(cycle);
            break;
        case CgiResponse::DOCUMENT_RES:
        case CgiResponse::CLIENT_REDIR_RES:
        case CgiResponse::CLIENT_REDIRDOC_RES:
            httpResponseHandler.makeHttpResponse(cycle, cgiResponseHandler.getCgiResponse());
            _setHttpResponseEvent(cycle);
            break;
        default:    /* in case of an error */
            httpRequestHandler.getHttpRequest().setCode(502);
            httpResponseHandler.makeHttpResponse(cycle, httpRequestHandler.getHttpRequest());
            _setHttpResponseEvent(cycle);
            break;
        }
    }
}

void EventHandler::_servCgiProc(const struct kevent &kev)
{
    pid_t pid = static_cast<pid_t>(kev.ident);

    waitpid(pid, NULL, WNOHANG);
    PidSet::erase(pid);
}

void EventHandler::_servFileRead(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();
    HttpResponse &httpResponse = httpResponseHandler.getHttpResponse();
    ssize_t readLen;

    if (cycle->beDeleted())
        return;

    if ((readLen = read(kev.ident, Cycle::getBuf(), std::min(static_cast<size_t>(kev.data), BUF_SIZE))) == FAILURE) {
        httpResponse.headerFields.clear();
        httpResponse.messageBody.clear();
        close(kev.ident);
        cycle->setReadFile(-1);
        _kqueueHandler.deleteEventType(kev.ident);
        if (httpResponseHandler.isErrorCode(httpResponse.statusLine.code)) {
            httpResponseHandler.makeHttpResponseFinal(cycle);
            _setHttpResponseEvent(cycle);
        }
        else {
            httpResponse.statusLine.code = 500;
            httpResponseHandler.makeErrorHttpResponse(cycle);
            _setHttpResponseEvent(cycle);
        }
        return;
    }

    if (cycle->getHttpRequestHandler().getHttpRequest().getRequestLine().getMethod() != HttpRequestHandler::HEAD) // HEAD인 경우 본문을 달지 않음
        httpResponse.messageBody.append(Cycle::getBuf(), readLen);

    if (readLen == kev.data) {
        close(kev.ident);
        cycle->setReadFile(-1);
        _kqueueHandler.deleteEventType(kev.ident);
        if (httpResponse.statusLine.code == 0)
            httpResponse.statusLine.code = 200;
        httpResponse.headerFields.insert(std::make_pair("Content-Length", toString(httpResponse.messageBody.size())));
        httpResponseHandler.makeHttpResponseFinal(cycle);
        _setHttpResponseEvent(cycle);
    }
}

void EventHandler::_servFileWrite(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();
    HttpResponse &httpResponse = httpResponseHandler.getHttpResponse();
    std::map<int, WriteFile> &writeFiles = cycle->getWriteFiles();
    std::map<int, WriteFile>::iterator it;

    if (cycle->beDeleted())
        return;

    if ((it = writeFiles.find(kev.ident)) == writeFiles.end())
        return;

    WriteFile &writeFile = it->second;

    if (writeFile.writeToFile(kev.ident) == FAILURE) {
        for (it = writeFiles.begin(); it != writeFiles.end(); ++it) {
            close(it->first);
            std::remove(it->second.getPath().c_str()); // 이미 파일은 만들어진 상태이다. open(O_CREAT)을 했기 때문.
            _kqueueHandler.deleteEventType(it->first);
        }
        writeFiles.clear();
        httpResponse.statusLine.code = 500;
        httpResponseHandler.makeErrorHttpResponse(cycle);
        _setHttpResponseEvent(cycle);
        return;
    }
    if (writeFile.eof()) {
        close(kev.ident);
        _kqueueHandler.deleteEventType(kev.ident);
        writeFiles.erase(it);
        if (writeFiles.empty()) {
            httpResponse.statusLine.code = 201;
            httpResponseHandler.makeHttpResponseFinal(cycle);
            _setHttpResponseEvent(cycle);
        }
    }
}

void EventHandler::_servSTimer(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();
    std::queue<HttpRequest> &httpRequestQ = cycle->getHttpRequestQueue();

    if (cycle->beDeleted() || cycle->closed())
        return;

    _kqueueHandler.deleteEvent(kev.ident, EVFILT_TIMER);
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
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);
    HttpRequestHandler &httpRequestHandler = cycle->getHttpRequestHandler();
    HttpResponseHandler &httpResponseHandler = cycle->getHttpResponseHandler();

    if (cycle->beDeleted())
        return;

    if (cycle->getCgiRecvfd() != -1) {
        if (PidSet::found(cycle->getCgiScriptPid()))
            kill(cycle->getCgiScriptPid(), SIGKILL);
        if (cycle->getCgiSendfd() != -1) {
            _kqueueHandler.deleteEventType(cycle->getCgiSendfd());
            close(cycle->getCgiSendfd());
            cycle->setCgiSendfd(-1);
        }
        _kqueueHandler.deleteEvent(cycle->getCgiRecvfd(), EVFILT_TIMER);
        _kqueueHandler.deleteEventType(cycle->getCgiRecvfd());
        close(cycle->getCgiRecvfd());
        cycle->setCgiRecvfd(-1);
        httpRequestHandler.getHttpRequest().setCode(504);
        httpResponseHandler.makeHttpResponse(cycle, httpRequestHandler.getHttpRequest());
        _setHttpResponseEvent(cycle);
    }
}

void EventHandler::_servError(const struct kevent &kev)
{
    Cycle *cycle = reinterpret_cast<Cycle *>(kev.udata);

    if (cycle != NULL)
        cycle->setBeDeleted();
}

void EventHandler::_destroyCycle(Cycle *cycle)
{
    _kqueueHandler.deleteEntry(cycle->getHttpSockfd(), EVFILT_READ);
    _kqueueHandler.deleteEntry(cycle->getHttpSockfd(), EVFILT_WRITE);
    _kqueueHandler.deleteEventType(cycle->getHttpSockfd());
    close(cycle->getHttpSockfd());
    if (!cycle->closed())
        _kqueueHandler.deleteEvent(cycle->getHttpSockfd(), EVFILT_TIMER);
    if (cycle->getCgiSendfd() != -1) {
        _kqueueHandler.deleteEventType(cycle->getCgiSendfd());
        close(cycle->getCgiSendfd());
    }
    if (cycle->getCgiRecvfd() != -1) {
        _kqueueHandler.deleteEvent(cycle->getCgiRecvfd(), EVFILT_TIMER);
        _kqueueHandler.deleteEventType(cycle->getCgiRecvfd());
        close(cycle->getCgiRecvfd());
    }
    if (cycle->getReadFile() != -1) {
        _kqueueHandler.deleteEventType(cycle->getReadFile());
        close(cycle->getReadFile());
    }
    for (std::map<int, WriteFile>::iterator it = cycle->getWriteFiles().begin(); it != cycle->getWriteFiles().end(); ++it) {
        _kqueueHandler.deleteEventType(it->first);
        close(it->first);
        std::remove(it->second.getPath().c_str());
    }
    if (PidSet::found(cycle->getCgiScriptPid()))
        kill(cycle->getCgiScriptPid(), SIGKILL);
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
    std::set<Cycle *> cycleBeDeleted;

    while (true) {
        _kqueueHandler.eventCatch();
        log("eventCatched");
        for (int i = 0; i < _kqueueHandler.getNevents(); ++i) {
            try {
                switch (_getEventType(eventList[i])) {
                case EVENT_LISTEN:
                    log("EVENT_LISTEN");
                    _servListen(eventList[i]);
                    break;
                case EVENT_HTTP_REQ:
                    log("EVENT_HTTP_REQ");
                    _servHttpRequest(eventList[i]);
                    break;
                case EVENT_HTTP_RES:
                    log("EVENT_HTTP_RES");
                    _servHttpResponse(eventList[i]);
                    break;
                case EVENT_CGI_REQ:
                    log("EVENT_CGI_REQ");
                    _servCgiRequest(eventList[i]);
                    break;
                case EVENT_CGI_RES:
                    log("EVENT_CGI_RES");
                    _servCgiResponse(eventList[i]);
                    break;
                case EVENT_CGI_PROC:
                    log("EVENT_CGI_PROC");
                    _servCgiProc(eventList[i]);
                    break;
                case EVENT_FILE_READ:
                    log("EVENT_FILE_READ");
                    _servFileRead(eventList[i]);
                    break;
                case EVENT_FILE_WRITE:
                    log("EVENT_FILE_WRITE");
                    _servFileWrite(eventList[i]);
                    break;
                case EVENT_STIMER:
                    log("EVENT_STIMER");
                    _servSTimer(eventList[i]);
                    break;
                case EVENT_CTIMER:
                    log("EVENT_CTIMER");
                    _servCTimer(eventList[i]);
                    break;
                case EVENT_ERROR:
                    log("EVENT_ERROR");
                    _servError(eventList[i]);
                    break;
                }
            }
            catch (std::runtime_error& e) {
                reinterpret_cast<Cycle*>(eventList[i].udata)->setBeDeleted();
            }
            if (eventList[i].udata != NULL && reinterpret_cast<Cycle *>(eventList[i].udata)->beDeleted()) {
                cycleBeDeleted.insert(reinterpret_cast<Cycle *>(eventList[i].udata));
            }
        }
        for (std::set<Cycle *>::iterator it = cycleBeDeleted.begin(); it != cycleBeDeleted.end(); ++it) {
            _destroyCycle(*it);
            Cycle::deleteCycle(*it);
        }
        cycleBeDeleted.clear();
    }
}
