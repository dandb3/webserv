#include <iostream>

#include "KqueueHandler.hpp"

// 클래스 초기화
KqueueHandler::KqueueHandler()
{
    _kqfd = kqueue();
    if (_kqfd == -1) {
        std::cerr << "kqueue() failed" << std::endl;
        exit(1);
    }
    _nevents = 0;
}

KqueueHandler::~KqueueHandler()
{
    close(_kqfd);
}

KqueueHandler::KqueueHandler(KqueueHandler const& ref) {}

KqueueHandler& KqueueHandler::operator=(KqueueHandler const& ref) {}

void KqueueHandler::addEvent(uintptr_t ident, int16_t filter, void* udata = NULL)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, EV_ADD, 0, 0, udata);
    _eventsToAdd.push_back(kev);
}

void KqueueHandler::deleteEvent(uintptr_t ident, int16_t filter, void* udata = NULL)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, EV_DELETE, 0, 0, udata);
    _eventsToAdd.push_back(kev);
}

void KqueueHandler::changeEvent(uintptr_t ident, int16_t filter, uint16_t flags, void* udata = NULL)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, flags, 0, 0, udata);
    _eventsToAdd.push_back(kev);
}

void KqueueHandler::eventCatch()
{
    int nev = kevent(_kqfd, &_eventsToAdd[0], _eventsToAdd.size(), _eventList, MAX_EVENTS, NULL);
    if (nev == -1) {
        std::cerr << "[eventCatch] : kevent() failed" << std::endl;
        exit(1);
    }
    _nevents = nev;
    _eventsToAdd.clear();
}

char KqueueHandler::getEventType(int ident)
{
    std::map<int, char>::iterator it = _type.find(ident);
    if (it == _type.end()) {
        std::cerr << "getEventType() failed" << std::endl;
        return -1;
    }
    return it->second;
}

void KqueueHandler::setEventType(int ident, char type)
{
    _type[ident] = type;
}

void KqueueHandler::deleteEventType(int ident)
{
    std::map<int, char>::iterator it = _type.find(ident);
    if (it == _type.end()) {
        std::cerr << "[deleteEventType] : there is no such event" << std::endl;
        return;
    }
    _type.erase(it);
}

struct kevent* KqueueHandler::getEventList() const
{
    return (struct kevent*)_eventList;
}

int KqueueHandler::getNevents() const
{
    return _nevents;
}