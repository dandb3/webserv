#include <iostream>

#include "ServerManager.hpp"

std::pair<std::string, int> getIpPort(std::string listen) {
    // std::string listen = it->getVariable("listen")[0];
    std::string ip;
    int port;
    size_t pos = listen.find(":");
    if (pos == std::string::npos) {
        if (listen.find(".") == std::string::npos) {
            ip = "";
            port = atoi(listen.c_str());
        }
        else {
            ip = listen;
            port = 80;
        }
    }
    else {
        ip = listen.substr(0, listen.find(":"));
        port = atoi(listen.substr(listen.find(":") + 1).c_str());
    }
    return std::make_pair(ip, port);
}

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
    Config& config = Config::getInstance();
    std::vector<ServerConfig>& server_v = config.getServerConfig();
    std::vector<ServerConfig>::iterator it = server_v.begin();
    for (; it != server_v.end(); it++) {
        std::pair<std::string, int> ip_port = getIpPort(it->getVariable("listen")[0]);
        int sockfd;
        struct sockaddr_in servaddr;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
            throw std::runtime_error("socket error");

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        if (ip_port.first == "")
            servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        else
            servaddr.sin_addr.s_addr = inet_addr(ip_port.first.c_str());
        servaddr.sin_port = htons(ip_port.second);

        // bind(): Address already in use 해결
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            throw std::runtime_error("setsockopt error");

        if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
            throw std::runtime_error("bind error");
        if (listen(sockfd, 10) == -1)
            throw std::runtime_error("listen error");
        if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
            throw std::runtime_error("fcntl error");
        _kqueue_handler.addEvent(sockfd, EVFILT_READ);
        _kqueue_handler.setEventType(sockfd, SOCKET_LISTEN);
    }
}
