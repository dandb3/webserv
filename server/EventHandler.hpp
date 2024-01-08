#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include "KqueueHandler.hpp"

class EventHandler
{
private:
    enum EventType_e
	{
		EVENT_LISTEN,
		EVENT_HTTP_REQ,
		EVENT_HTTP_RES,
		EVENT_CGI_REQ,
		EVENT_CGI_RES,
		EVENT_ERROR,
	};

    KqueueHandler _kqueueHandler;

    

    void _servListen();
    void _servHttpRequest();
    void _servHttpResponse();
    void _servCgiRequest();
    void _servCgiResponse();
    void _servError();

public:
    void operate();

};

#endif
