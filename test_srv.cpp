#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <stdexcept>
#include "./srcs/server/modules/HttpRequestModule.hpp"
#include "./srcs/server/modules/HttpResponseModule.hpp"
#include "./srcs/server/cycle/ConfigInfo.hpp"
using namespace std;

int main(int ac, char **av) {
    // 서버 소켓 생성
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket." << std::endl;
        return 1;
    }

    int port = av[1] ? atoi(av[1]) : 12345;

    // 서버 주소 설정
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // 소켓을 주소에 바인딩
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding server socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // 클라이언트 연결 대기
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening for clients." << std::endl;
        close(serverSocket);
        return 1;
    }

    // 클라이언트 연결 수락
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1) {
        std::cerr << "Error accepting client connection." << std::endl;
        close(serverSocket);
        return 1;
    }

    // 클라이언트로부터 메시지 수신
    std::queue<HttpRequest> que;
    char buffer[1024];

    HttpRequestHandler requestHandler;

    requestHandler.recvHttpRequest(clientSocket, 86);
    requestHandler.parseHttpRequest(0, que);
    HttpRequest httpRequest = que.front(); que.pop();
    std::string uri = httpRequest.getRequestLine().getUri(); // uri
    struct sockaddr_in servaddr;
    socklen_t servLen = sizeof(servaddr);
    if (getsockname(clientSocket, (struct sockaddr *) &servaddr, &servLen) == -1) {
        std::cerr << "Error getting server socket name." << std::endl;
        close(clientSocket);
        close(serverSocket);
        return 1;
    }
    // 연결된 서버의 port 구하기
    int requestPort = ntohs(servaddr.sin_port); // port

    in_addr_t ip = servaddr.sin_addr.s_addr; // ip
    ConfigInfo configInfo(ip, requestPort, uri);
    // configInfo.printConfigInfo();

    HttpResponseHandler responseHandler;

    responseHandler.makeHttpResponse(que.front(), configInfo);

    // std::cout << que.size() << '\n';
    // ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    // if (bytesRead == -1) {
    //     std::cerr << "Error receiving message from client." << std::endl;
    // } else {
    //     buffer[bytesRead] = '\0';
    //     std::cout << "Received message from client: " << buffer << std::endl;
    // }

    // 클라이언트 소켓 닫기
    close(clientSocket);

    // 서버 소켓 닫기
    close(serverSocket);

    return 0;
}
