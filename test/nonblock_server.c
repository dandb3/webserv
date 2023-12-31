#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>

void err(const char *msg);
void sockaddr_init(struct sockaddr_in *addr, u_int16_t port, u_int32_t ip);

int main(int argc, char *argv[])
{
	int sockfd, new_sockfd, kq, n;
	socklen_t peer_len = sizeof(struct sockaddr_in);
	struct sockaddr_in addr, peer_addr;
	struct kevent changelist[2];
	struct kevent eventlist[10];
	struct timespec timeout =
	{
		.tv_nsec = 0,
		.tv_sec = 0,
	};
	char buf[1024];

	if (argc != 1)
		return 1;
	memset(buf, 0, sizeof(buf));
	memset(&peer_addr, 0, sizeof(peer_addr));
	sockaddr_init(&addr, 10002, INADDR_ANY);
	EV_SET(changelist, sockfd, EVFILT_READ, EV_ADD, 0, 0, 0);
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		err("socket failed");
	if (bind(sockfd, &addr, sizeof(addr)) == -1)
		err("bind failed");
	if (listen(sockfd, 100) == -1)
		err("listen failed");
	new_sockfd = accept(sockfd, &peer_addr, &peer_len);
	//fcntl(new_sockfd, F_SETFL, O_NONBLOCK);
	int status = read(new_sockfd, buf, sizeof(buf));
	printf("status: %d\n", status);
	printf("%s", buf);
	close(new_sockfd);
	close(sockfd);
	return 0;
}

void sockaddr_init(struct sockaddr_in *addr, u_int16_t port, u_int32_t ip)
{
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = htonl(ip);
}

void err(const char *msg)
{
	perror(msg);
	exit(1);
}

	// int sockfd2;
	// struct sockaddr_in addr2;
	// memset(&addr2, 0, sizeof(addr2));
	// addr2.sin_family = AF_INET;
	// addr.sin_port = htons(9999);
	// addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	// if ((sockfd2 = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	// 	err("socket failed");
	// if (bind(sockfd2, &addr, sizeof(addr)) == -1)
	// 	err("bind failed2");
//	printf("bind successfully completed\n");