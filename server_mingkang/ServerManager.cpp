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

void printServerInfo(ServerConfig &server)
{
    std::cout << "======================" << std::endl;
    std::cout << "server name: " << server.getServerName()[0] << std::endl;
    for (auto it2 = server.getServerInfo().begin(); it2 != server.getServerInfo().end(); it2++) {
        std::cout << it2->first << ": ";
        for (auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
            std::cout << *it3 << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "======================" << std::endl;
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
    for (; it != server_v.end(); it++) {
        // getVariable 실패 시 어떻게 처리할지 고민
        // default 값 설정해서 실패 안나게 할까? or 예외 처리?

        // it 값 확인
        // printServerInfo(*it);
        int sockfd;
        struct sockaddr_in servaddr;
        // 해당 port의 "0.0.0.0" ip가 있는 경우 -> 서버 생성하지 않고 넘어가기
        if (std::find(it->portsWithINADDR_ANY.begin(), it->portsWithINADDR_ANY.end(), it->getPort()) != it->portsWithINADDR_ANY.end() && \
            it->getIp().s_addr != INADDR_ANY)
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
        _kqueue_handler.addEvent(sockfd, EVFILT_READ);
        _kqueue_handler.setEventType(sockfd, SOCKET_LISTEN);
    }
}

/*
** 서버 동작 함수
** 임시로 에코 서버 구현
** 나중에는 이벤트 핸들러에서 이벤트를 받아와서 처리할 예정
**
*/
void ServerManager::operate()
{
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
            std::cout << "type: " << static_cast<int>(type) << std::endl;
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
                if (type == SOCKET_LISTEN) {
                    struct sockaddr_in cliaddr;
                    socklen_t cliLen = sizeof(cliaddr);
                    int clientSocket = accept(sockfd, (struct sockaddr *)&cliaddr, &cliLen);
                    if (clientSocket == -1)
                        throw std::runtime_error("accept error");
                    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1)
                        throw std::runtime_error("fcntl error");
                    _kqueue_handler.addEvent(clientSocket, EVFILT_READ, (void *)"");
                    _kqueue_handler.setEventType(clientSocket, SOCKET_CLIENT);
                }
                else if (type == SOCKET_CLIENT) {
                    char buf[1024];
                    int n = read(sockfd, buf, 1024);
                    if (n == -1)
                        throw std::runtime_error("read error");
                    else if (n == 0) {
                        _kqueue_handler.deleteEvent(sockfd, EVFILT_READ);
                        _kqueue_handler.deleteEventType(sockfd);
                        close(sockfd);
                    }
                    else {
                        buf[n] = '\0';
                        // Cast std::string to const char* before assigning to curEvent.udata
                        curEvent.udata = (void *)(std::string((char *)curEvent.udata) + buf).c_str();
                        _kqueue_handler.changeEvent(sockfd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, curEvent.udata);
                        // write(sockfd, buf, n);
                    }
                }
            }
            else if (curEvent.filter == EVFILT_WRITE) {
                std::cout << "EVFILT_WRITE" << std::endl;
                if (type == SOCKET_CLIENT) {
                    std::string data = (char *)curEvent.udata;
                    int n = write(sockfd, data.c_str(), data.length());
                    if (n == -1)
                        throw std::runtime_error("write error");
                    else if (n != data.length()) {
                        curEvent.udata = (void *)(data.substr(n)).c_str();
                    }
                }
            }
        }
    }
}
