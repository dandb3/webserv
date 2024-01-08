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

void KqueueHandler::changeEvent(uintptr_t ident, int16_t filter, uint16_t flags, void *udata)
{
    struct kevent kev;

    EV_SET(&kev, ident, filter, flags, 0, 0, udata);
    _eventsToAdd.push_back(kev);
}

void KqueueHandler::eventCatch()
{
    // // kevent 전에 KqueueHandler에 대한 정보 출력
    // std::cout << "== KqueueHandler ==" << std::endl;
    // std::cout << "kqfd: " << _kqfd << std::endl;
    // std::cout << "nevents: " << _nevents << std::endl;
    // std::cout << "eventsToAdd: " << _eventsToAdd.size() << std::endl;
    // for (int i = 0; i < _eventsToAdd.size(); i++) {
    //     std::cout << "ident: " << _eventsToAdd[i].ident << std::endl;
    //     std::cout << "filter: " << _eventsToAdd[i].filter << std::endl;
    //     std::cout << "flags: " << _eventsToAdd[i].flags << std::endl;
    //     std::cout << "fflags: " << _eventsToAdd[i].fflags << std::endl;
    //     std::cout << "data: " << _eventsToAdd[i].data << std::endl;
    //     std::cout << "udata: " << _eventsToAdd[i].udata << std::endl;
    // }
    // std::cout << "eventList: " << std::endl;
    // for (int i = 0; i < _nevents; i++) {
    //     std::cout << "ident: " << _eventList[i].ident << std::endl;
    //     std::cout << "filter: " << _eventList[i].filter << std::endl;
    //     std::cout << "flags: " << _eventList[i].flags << std::endl;
    //     std::cout << "fflags: " << _eventList[i].fflags << std::endl;
    //     std::cout << "data: " << _eventList[i].data << std::endl;
    //     std::cout << "udata: " << _eventList[i].udata << std::endl;
    // }
    // std::cout << "type: " << std::endl;
    // for (std::map<int, char>::iterator it = _type.begin(); it != _type.end(); it++) {
    //     std::cout << "ident: " << it->first << ", type: " << static_cast<int>(it->second) << std::endl;
    // }
    // std::cout << std::endl;
    // std::cout << "===================" << std::endl;
    int nev = kevent(_kqfd, &_eventsToAdd[0], _eventsToAdd.size(), _eventList, MAX_EVENTS, NULL);
    if (nev == -1) {
        std::cerr << "[eventCatch] : kevent() failed" << std::endl;
        if (errno == EAGAIN) {
            std::cerr << "[eventCatch] : EAGAIN" << std::endl;
        }
        exit(1);
    }
    _nevents = nev;
    _eventsToAdd.clear();
}

char KqueueHandler::getEventType(int ident)
{
    // std::map<int, char>::iterator it = _type.find(ident);
    // if (it == _type.end()) {
    //     std::cerr << "getEventType() failed" << std::endl;
    //     return -1;
    // }
    // return it->second;
    char type = _type[ident];
    if (type == 0) {
        std::cerr << "[getEventType] : there is no such event" << std::endl;
        return -1;
    }
    return type;
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
