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
    // _eventsToAdd에서 해당 이벤트를 찾아 삭제
    std::vector<struct kevent>::iterator it = _eventsToAdd.begin();
    while (it != _eventsToAdd.end()) {
        if (it->ident == ident && it->filter == filter) {
            // _eventsToAdd에서 해당 이벤트를 삭제
            it = _eventsToAdd.erase(it);
            break;
        }
        else
            ++it;   // 다음 이벤트로 이동
    }
    // 커널에게 이벤트를 감시하지 않도록 지시
    struct kevent kev;
    EV_SET(&kev, ident, filter, EV_DELETE, 0, 0, udata);
    kevent(_kqfd, &kev, 1, NULL, 0, NULL);
}

void KqueueHandler::eventCatch()
{
    int nev = kevent(_kqfd, &_eventsToAdd[0], _eventsToAdd.size(), _eventList, MAX_EVENTS, NULL);
    if (nev == -1) {
        std::cerr << "[eventCatch] : kevent() failed" << std::endl;
        exit(1);
    }
    _nevents = nev;
    _eventsToAdd.clear(); // _eventsToAdd를 비움. 맞는지 모르겠음
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
