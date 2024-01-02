#ifndef KQUEUEHANDLER_HPP
#define KQUEUEHANDLER_HPP

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

#include "../config_parsing/Config.hpp"

#define MAX_EVENTS 20

/*
**  KqueueHandler
**  Kqueue에 대한 이벤트를 처리하는 클래스
**
*/
class KqueueHandler
{
private:
    enum SocketType_e
    {
        SOCKET_LISTEN,
        SOCKET_CLIENT,
        SOCKET_CGI
    };

    /* data */
    int _kqfd;
    int _nevents;
    std::vector<struct kevent> _eventsToMonitor;
    struct kevent _eventList[MAX_EVENTS];
    std::map<int, char> _type;

    KqueueHandler(const KqueueHandler &ref);
    KqueueHandler &operator=(const KqueueHandler &ref);

public:
    KqueueHandler();
    ~KqueueHandler();

    /* method */
    void addEvent(uintptr_t ident, int16_t filter, void *udata = NULL);
    void deleteEvent(uintptr_t ident, int16_t filter, void *udata = NULL);
    void eventCatch();
    // void enableEvent(/* args */);
    // void disableEvent(/* args */);
    char getEventType(int ident);
    void setEventType(int ident, char type);
    void deleteEventType(int ident);
};

#endif
