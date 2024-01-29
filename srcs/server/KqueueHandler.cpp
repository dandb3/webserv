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

KqueueHandler::KqueueHandler(const KqueueHandler &ref) {
    _kqfd = ref._kqfd;
    _nevents = ref._nevents;
    _eventsToAdd = ref._eventsToAdd;
    std::copy(ref._eventList, ref._eventList + MAX_EVENTS, _eventList);
    _type = ref._type;
}

KqueueHandler &KqueueHandler::operator=(const KqueueHandler &ref) {
    if (this != &ref) {
        _kqfd = ref._kqfd;
        _nevents = ref._nevents;
        _eventsToAdd = ref._eventsToAdd;
        std::copy(ref._eventList, ref._eventList + MAX_EVENTS, _eventList);
        _type = ref._type;
    }
    return *this;
}

void KqueueHandler::addEvent(uintptr_t ident, int16_t filter, void *udata)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, EV_ADD, 0, 0, udata);
    _eventsToAdd.push_back(kev);
}

void KqueueHandler::deleteEvent(uintptr_t ident, int16_t filter, void *udata)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, EV_DELETE, 0, 0, udata);
    _eventsToAdd.push_back(kev);
}

void KqueueHandler::changeEvent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags = 0, intptr_t data = 0, void *udata = NULL)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, flags, fflags, data, udata);
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
    if (_type.find(ident) == _type.end()) {
        std::cerr << "[getEventType] : there is no such event" << std::endl;
        return -1;
    }
    return _type[ident];
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

struct kevent *KqueueHandler::getEventList() const
{
    return (struct kevent *)_eventList;
}

int KqueueHandler::getNevents() const
{
    return _nevents;
}
