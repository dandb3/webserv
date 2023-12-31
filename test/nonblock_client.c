#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

void err(const char *msg);

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in addr;
	char buf[1024];

	if (argc != 1)
		return 1;
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		err("socket failed");
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10002);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
//	printf("socket successfully completed\n");
	if (connect(sockfd, &addr, sizeof(addr)) == -1)
		err("connect failed");
	write(sockfd, buf, read(0, buf, sizeof(buf)));
	close(sockfd);
	return 0;
}

void err(const char *msg)
{
	perror(msg);
	exit(1);
}