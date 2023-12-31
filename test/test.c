#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void printSocketInfo(int sockfd) {
    struct sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);

    // getsockname()을 호출하여 소켓이 바인딩된 정보를 출력
    if (getsockname(sockfd, (struct sockaddr*)&localAddr, &addrLen) == 0) {
        printf("Initial Socket Binding Info: %s:%d\n", inet_ntoa(localAddr.sin_addr), ntohs(localAddr.sin_port));
    } else {
        perror("getsockname");
    }
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(10000);

    // 소켓을 특정 주소에 바인딩
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 초기 바인딩 정보 출력
    printSocketInfo(sockfd);

    // 실제로는 여기서 연결을 대기하거나 데이터를 받아야 함
    // 여기서는 간단히 예시로 메시지 수신 대기를 하는 것으로 가정
    listen(sockfd, 1);
    int clientSock = accept(sockfd, NULL, NULL);

    char buffer[1024];
    ssize_t bytesRead = recv(clientSock, buffer, sizeof(buffer), 0);
    buffer[bytesRead] = '\0';

    printf("Received message: %s\n", buffer);

    // 데이터를 수신한 후에 갱신된 바인딩 정보 출력
    printSocketInfo(sockfd);

    // 소켓 닫기
    close(sockfd);

    return 0;
}
