#ifndef KQUEUE_HANDLER_HPP
#define KQUEUE_HANDLER_HPP

#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <vector>
#include <iostream>
#include <map>

#include "../config/Config.hpp"

#define MAX_EVENTS 20

/*
**  KqueueHandler
*/
class KqueueHandler
{
public:
	enum SocketType_e
	{
		SOCKET_LISTEN,
		SOCKET_CLIENT,
		SOCKET_CGI,
        FILE_OPEN
	};

private:
    /* data */
    int _kqfd;
    int _nevents;
    std::map<std::pair<uintptr_t, int16_t>, struct kevent> _eventsToAdd;
    struct kevent _eventList[MAX_EVENTS];
    std::map<int, char> _type;

public:
    KqueueHandler();
    ~KqueueHandler();
    KqueueHandler(const KqueueHandler &ref);
    KqueueHandler &operator=(const KqueueHandler &ref);

    /* method */
    void addEvent(uintptr_t ident, int16_t filter, void *udata = NULL);
    void deleteEvent(uintptr_t ident, int16_t filter, void *udata = NULL);
    void changeEvent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags = 0, intptr_t data = 0, void *udata = NULL);
    void deleteEntry(uintptr_t ident, int16_t filter);
    void eventCatch();
    char getEventType(int ident);
    void setEventType(int ident, char type);
    void deleteEventType(int ident);

    /* getter */
    struct kevent *getEventList() const;
    int getNevents() const;
};

#endif
