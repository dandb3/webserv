#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <fcntl.h>

#define BUF_SIZE 4

void setnonblockingmode(int fd);
void error_handling(const std::string &message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    int str_len;

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <IP> <port>" << std::endl;
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    setnonblockingmode(STDIN_FILENO); // 표준 입력도 논블록 모드로 설정
    setnonblockingmode(sock);

    int kq = kqueue();
    struct kevent event, events[2];

    EV_SET(&event, STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    kevent(kq, &event, 1, nullptr, 0, nullptr);

    EV_SET(&event, sock, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    kevent(kq, &event, 1, nullptr, 0, nullptr);

    while (1)
    {
        int event_cnt = kevent(kq, nullptr, 0, events, 2, nullptr);
        if (event_cnt == -1)
        {
            std::cerr << "kevent() error" << std::endl;
            break;
        }

        for (int i = 0; i < event_cnt; i++)
        {
            if (events[i].ident == STDIN_FILENO)
            {
                str_len = read(STDIN_FILENO, buf, BUF_SIZE);
                if (str_len == 0)
                {
                    close(sock);
                    return 0;
                }
                else if (str_len > 0)
                {
                    write(sock, buf, str_len);
                }
            }

            if (events[i].ident == sock)
            {
                str_len = read(sock, buf, BUF_SIZE);
                if (str_len == 0)
                {
                    std::cout << "Server disconnected." << std::endl;
                    close(sock);
                    return 0;
                }
                else if (str_len > 0)
                {
                    write(STDOUT_FILENO, buf, str_len);
                }
            }
        }
    }

    return 0;
}

void setnonblockingmode(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void error_handling(const std::string &message)
{
    std::cerr << message << std::endl;
    exit(EXIT_FAILURE);
}
