#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <vector>
#include <map>
#include "Converter.hpp"

void add_event(std::vector<struct kevent>& v, int ident, int filter, int flags);
void err(const char *msg);

void operate()
{
    int sockfd, new_sockfd, kq, n;
    int nevents;
	socklen_t peer_len = sizeof(struct sockaddr_in);
	struct sockaddr_in addr, peer_addr;
    std::vector<struct kevent> changelist(1);
	struct kevent eventlist[10];
	struct timespec timeout =
	{
		.tv_nsec = 0,
		.tv_sec = 0,
	};
    std::map<int, Converter> m;

    memset(&peer_addr, 0, sizeof(peer_addr));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    EV_SET(&changelist[0], sockfd, EVFILT_READ, EV_ADD, 0, 0, 0);
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        err("socket failed");
    if (bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
        err("bind failed");
    if (listen(sockfd, 100) == -1)
        err("listen failed");

    if ((kq = kqueue()) == -1)
        err("kqueue failed");
    while (true) {
        nevents = kevent(kq, &changelist[0], changelist.size(), eventlist, 10, &timeout);
        changelist.clear();
        for (int i = 0; i < nevents; ++i) {
            if (eventlist[i].ident == sockfd) {
                new_sockfd = accept(sockfd, reinterpret_cast<sockaddr*>(&peer_addr), &peer_len);
                add_event(changelist, new_sockfd, EVFILT_READ, EV_ADD);
                m[new_sockfd];
            }
            else if (eventlist[i].filter == EVFILT_READ) {
                Converter& target = m[eventlist[i].ident];
                target.add(eventlist[i].ident);
                if (target.eof()) {
                    add_event(changelist, eventlist[i].ident, EVFILT_WRITE, EV_ADD);
                    add_event(changelist, eventlist[i].ident, EVFILT_READ, EV_DELETE);
                }
            }
            else if (eventlist[i].filter == EVFILT_WRITE) {
                Converter& target = m[eventlist[i].ident];
                target.response(eventlist[i].ident);
                close(eventlist[i].ident);
                m.erase(eventlist[i].ident);
            }
            else {
                err("ERRROROROROROROR");
            }
        }
    }
}

void add_event(std::vector<struct kevent>& v, int ident, int filter, int flags)
{
    struct kevent new_event;
    EV_SET(&new_event, ident, filter, flags, 0, 0, 0);
    v.push_back(new_event);
}

void err(const char *msg)
{
	perror(msg);
	exit(1);
}
