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
// #include "./srcs/server/modules/HttpRequestModule.hpp"
// #include "./srcs/server/modules/HttpResponseModule.hpp"
#include <cstring>
#include <cstdlib>
#include <unistd.h>
using namespace std;

int main(int ac, char **av) {
    // 클라이언트 소켓 생성
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating client socket." << std::endl;
        return 1;
    }

    int port = av[1] ? atoi(av[1]) : 12345;

    // 서버 주소 설정
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP 주소
    serverAddress.sin_port = htons(port); // 서버 포트 번호

    // 서버에 연결
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error connecting to server." << std::endl;
        close(clientSocket);
        return 1;
    }

    // 서버로 메시지 전송
    int fd = open("test1.txt", O_RDONLY);
    char message[1024];
    memset(message, 0, 1024);
    std::cout << read(fd, message, 1024) << '\n';

    if (send(clientSocket, message, strlen(message), 0) == -1) {
        std::cerr << "Error sending message to server." << std::endl;
    } else {
        std::cout << "Message sent to server: " << message << std::endl;
    }

    // 클라이언트 소켓 닫기
    close(clientSocket);

    return 0;
}
