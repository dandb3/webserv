#include <sys/event.h>
#include "webserv.hpp"
#include "EventHandler.hpp"

void EventHandler::operate()
{
    struct kevent* eventList = _kqueueHandler.getEventList();

    while (true) {
        _kqueueHandler.eventCatch();
        for (int i = 0; i < _kqueueHandler.getNevents(); ++i) {
            switch (_kqueueHandler.getEventType(eventList[i].ident)) {
            case EVENT_LISTEN:
                
                break;
            case EVENT_HTTP_REQ:
                break;
            case EVENT_HTTP_RES:
                break;
            case EVENT_CGI_REQ:
                break;
            case EVENT_CGI_RES:
                break;
            case EVENT_ERROR:
                break;
            }
        }
    }
}
