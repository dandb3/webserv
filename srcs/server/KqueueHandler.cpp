#include <iostream>
#include "KqueueHandler.hpp"

// 클래스 초기화
KqueueHandler::KqueueHandler()
{
    _kqfd = kqueue();
    if (_kqfd == -1)
        throw std::runtime_error("kqueue() failed");
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
    _eventsToAdd[std::make_pair(ident, filter)] = kev;
}

void KqueueHandler::deleteEvent(uintptr_t ident, int16_t filter, void *udata)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, EV_DELETE, 0, 0, udata);
    _eventsToAdd[std::make_pair(ident, filter)] = kev;
}

void KqueueHandler::changeEvent(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, flags, fflags, data, udata);
    _eventsToAdd[std::make_pair(ident, filter)] = kev;
}

void KqueueHandler::deleteEntry(uintptr_t ident, int16_t filter)
{
    _eventsToAdd.erase(std::make_pair(ident, filter));
}

void KqueueHandler::eventCatch()
{
    std::vector<struct kevent> v;
    std::map<std::pair<uintptr_t, int16_t>, struct kevent>::iterator it;

    for (it = _eventsToAdd.begin(); it != _eventsToAdd.end(); ++it)
        v.push_back(it->second);
    int nev = kevent(_kqfd, &v[0], v.size(), _eventList, MAX_EVENTS, NULL);
    if (nev == -1)
        throw std::runtime_error("[eventCatch] : kevent() failed");
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
