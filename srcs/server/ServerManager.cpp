#include <iostream>
#include "ServerManager.hpp"

ServerManager::ServerManager()
{
    Config::getInstance();
}

ServerManager::ServerManager(std::string config_path)
{
    Config::getInstance(config_path);
}
/**
 * @brief 서버 초기화 함수
 * @details
 *    - Config에서 서버 설정을 가져와 각 서버를 초기화합니다.
 *    - 소켓을 생성하고 주소에 바인딩합니다.
 *    - 소켓을 리스닝 모드로 설정합니다.
 *    - 소켓을 논블로킹 모드로 설정하고 이벤트 핸들러에 이벤트를 추가합니다.
 *    - 서버 소켓 타입을 설정합니다.
 * @throws std::runtime_error 소켓 생성, 주소 바인딩, 논블로킹 설정, 이벤트 추가 등의 과정에서 오류 발생 시 예외 처리
 *      예외 처리는 나중에 구현할 예정입니다.
 */
void ServerManager::initServer()
{
    Config &config = Config::getInstance();
    std::vector<ServerConfig> &server_v = config.getServerConfig();
    std::vector<ServerConfig>::iterator it = server_v.begin();
    std::set<std::pair<in_addr_t, int> > server_set;
    std::vector<int> listenFds;

    for (; it != server_v.end(); it++) {
        // getVariable 실패 시 어떻게 처리할지 고민
        // default 값 설정해서 실패 안나게 할까? or 예외 처리?

        int sockfd;
        struct sockaddr_in servaddr;
        // 해당 port의 "0.0.0.0" ip가 있는 경우 -> 서버 생성하지 않고 넘어가기
        if (std::find(it->globalPortList.begin(), it->globalPortList.end(), it->getPort()) != it->globalPortList.end() && \
            it->getIp().s_addr != INADDR_ANY)
            continue;

        // 이미 해당 ip, port로 서버가 생성된 경우 -> 서버 생성하지 않고 넘어가기
        if (server_set.find(std::make_pair(it->getIp().s_addr, it->getPort())) != server_set.end())
            continue;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
            throw std::runtime_error("socket error");
        std::cout << "socket: " << sockfd << std::endl;

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = (it->getIp()).s_addr;
        servaddr.sin_port = htons(it->getPort());

        // bind(): Address already in use 해결
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            throw std::runtime_error("setsockopt error");

        if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
            throw std::runtime_error("bind error");
        if (listen(sockfd, 10) == -1)
            throw std::runtime_error("listen error");
        if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
            throw std::runtime_error("fcntl error");
        server_set.insert(std::make_pair(it->getIp().s_addr, it->getPort()));
        listenFds.push_back(sockfd);
    }
    _eventHandler.initEvent(listenFds);
}

std::string createHttpRequestPage(const std::string &httpRequest) {
    // HTTP 요청 헤더와 본문을 추출
    std::string headers;
    std::string body;

    size_t pos = httpRequest.find("\r\n\r\n");
    if (pos != std::string::npos) {
        headers = httpRequest.substr(0, pos);
        body = httpRequest.substr(pos + 4);
    }

    // HTML 페이지 생성
    std::string htmlContent = R"(
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>HTTP Request Page</title>
        </head>
        <body>
            <header>
                <h1>HTTP Request Content</h1>
            </header>
            <section>
                <h2>Request Headers</h2>
                <pre>)" + headers + R"(</pre>
                <h2>Request Body</h2>
                <pre>)" + body + R"(</pre>
            </section>
            <footer>
                <p>&copy; 2024 My Website. All rights reserved.</p>
            </footer>
        </body>
        </html>
    )";

    // HTTP 헤더
    std::string httpResponse;
    httpResponse += "HTTP/1.1 200 OK\r\n";
    httpResponse += "Content-Type: text/html\r\n";
    httpResponse += "Content-Length: " + std::to_string(htmlContent.length()) + "\r\n";
    httpResponse += "\r\n"; // 헤더와 본문을 구분하는 빈 줄

    // HTTP 본문
    httpResponse += htmlContent;

    return httpResponse;
}

/*
** 서버 동작 함수
** 임시로 에코 서버 구현
** 나중에는 이벤트 핸들러에서 이벤트를 받아와서 처리할 예정
**
*/
void ServerManager::operate()
{
    struct EventInfo
    {
        int sockfd;
        std::string type;
        std::string data;
    };
    KqueueHandler _kqueue_handler = _eventHandler.getKqueueHandler();
    while (true) {
        _kqueue_handler.eventCatch();
        int nevents = _kqueue_handler.getNevents();
        std::cout << "nevents: " << nevents << std::endl;
        struct kevent *event_list = _kqueue_handler.getEventList();
        for (int i = 0; i < nevents; i++) {
            struct kevent curEvent = event_list[i];
            int sockfd = curEvent.ident;
            std::cout << "ident: " << sockfd << std::endl;
            char type = _kqueue_handler.getEventType(sockfd);
            if (curEvent.flags & EV_ERROR) {
                std::cerr << "EV_ERROR" << std::endl;
                if (_kqueue_handler.getEventType(sockfd) == SOCKET_LISTEN) {
                    std::cerr << "listen error" << std::endl;
                    _kqueue_handler.deleteEventType(sockfd);
                    close(sockfd);
                }
                else {
                    std::cerr << "client error" << std::endl;
                    _kqueue_handler.deleteEventType(sockfd);
                    close(sockfd);
                }
            }
            else if (curEvent.filter == EVFILT_READ) {
                std::cout << "EVFILT_READ" << std::endl;
                if (type == _kqueue_handler.SOCKET_LISTEN) {
                    std::cout << "SOCKET_LISTEN" << std::endl;
                    struct sockaddr_in cliaddr;
                    socklen_t cliLen = sizeof(cliaddr);
                    int clientSocket = accept(sockfd, (struct sockaddr *)&cliaddr, &cliLen);
                    if (clientSocket == -1)
                        throw std::runtime_error("accept error");
                    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1)
                        throw std::runtime_error("fcntl error");
                    _kqueue_handler.addEvent(clientSocket, EVFILT_READ, (void *)"");
                    _kqueue_handler.setEventType(clientSocket, _kqueue_handler.SOCKET_CLIENT);
                }
                else if (type == _kqueue_handler.SOCKET_CLIENT) {
                    char buf[1024];
                    int n = read(sockfd, buf, 1024);
                    if (n == -1) {
                        throw std::runtime_error("read error");
                    }
                    else if (n == 0) {
                        std::cerr << "clients disconnected" << std::endl;
                        _kqueue_handler.deleteEventType(sockfd);
                        close(sockfd);
                    }
                    else {
                        buf[n] = '\0';
                        EventInfo *curEventInfo = new EventInfo;
                        curEventInfo->sockfd = sockfd;
                        curEventInfo->type = (type == _kqueue_handler.SOCKET_LISTEN) ? "listen" : "client";
                        curEventInfo->data = curEventInfo->data + buf;
                        curEvent.udata = (void *)curEventInfo;
                        _kqueue_handler.changeEvent(sockfd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, curEvent.udata);
                    }
                }
            }
            else if (curEvent.filter == EVFILT_WRITE) {
                std::cout << "EVFILT_WRITE" << std::endl;
                if (type == _kqueue_handler.SOCKET_CLIENT) {
                    EventInfo *data = (EventInfo *)curEvent.udata;
                    // EventInfo에 대한 fd string으로 변환
                    std::string fd = std::to_string(data->sockfd);

                    std::string msg = createHttpRequestPage(data->data);
                    int n = write(sockfd, msg.c_str(), msg.length());
                    if (n == -1) {
                        std::cerr << "write error" << std::endl;
                        std::cerr << "clients disconnected" << std::endl;
                        delete (EventInfo *)curEvent.udata;
                        _kqueue_handler.deleteEvent(sockfd, EVFILT_WRITE);
                        _kqueue_handler.deleteEventType(sockfd);
                        close(sockfd);
                    }
                    else { // udata 삭제
                        delete (EventInfo *)curEvent.udata;
                    }

                }
            }
        }
    }
}
