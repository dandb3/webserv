#include <iostream>
#include <csignal>
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
 *    - SIGPIPE 시그널 발생 시 SIG_IGN으로 설정합니다.
 * @throws std::runtime_error 소켓 생성, 주소 바인딩, 논블로킹 설정, 이벤트 추가 등의 과정에서 오류 발생 시 예외 처리
 */
void ServerManager::initServer()
{
    Config &config = Config::getInstance();
    std::vector<ServerConfig> &serverVec = config.getServerConfig();
    std::vector<ServerConfig>::iterator it = serverVec.begin();
    std::set<std::pair<in_addr_t, int> > serverSet;
    std::vector<int> listenFds;

    for (; it != serverVec.end(); it++) {
        int sockfd;
        struct sockaddr_in servaddr;
        // 해당 port의 "0.0.0.0" ip가 있는 경우 -> 서버 생성하지 않고 넘어가기
        if (std::find(it->globalPortList.begin(), it->globalPortList.end(), it->getPort()) != it->globalPortList.end() && \
            it->getIp().s_addr != INADDR_ANY)
            continue;

        // 이미 해당 ip, port로 서버가 생성된 경우 -> 서버 생성하지 않고 넘어가기
        if (serverSet.find(std::make_pair(it->getIp().s_addr, it->getPort())) != serverSet.end())
            continue;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
            throw std::runtime_error("socket error");

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = (it->getIp()).s_addr;
        servaddr.sin_port = htons(it->getPort());

        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            throw std::runtime_error("setsockopt error");

        if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
            throw std::runtime_error("bind error");
        if (listen(sockfd, 1000) == -1)
            throw std::runtime_error("listen error");
        if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
            throw std::runtime_error("fcntl error");
        serverSet.insert(std::make_pair(it->getIp().s_addr, it->getPort()));
        listenFds.push_back(sockfd);
    }
    _eventHandler.initEvent(listenFds);
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        throw std::runtime_error("signal error");
}

void ServerManager::operate()
{
    initServer();
    _eventHandler.operate();
}
