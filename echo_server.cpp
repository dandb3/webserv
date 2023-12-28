#include <iostream>
#include <cstring>
#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

void error_handling(const char* msg) {
    throw std::runtime_error(msg);
}

int main() {
    // 소켓 생성
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // 서버 소켓 설정
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // 바인딩
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    // 대기 상태 진입
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // 클라이언트 연결 수락 및 에코
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    while (true) {
        // 클라이언트 연결 수락
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            close(serverSocket);
            return -1;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;

        // 데이터 수신 및 에코
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            send(clientSocket, buffer, bytesRead, 0);
        }

        // 클라이언트 소켓 닫기
        close(clientSocket);
    }

    // 서버 소켓 닫기
    close(serverSocket);

    return 0;
}
