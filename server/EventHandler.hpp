#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include "KqueueHandler.hpp"

class EventHandler
{
private:
    KqueueHandler _kqueueHandler;

public:
    void operate();

};

#endif
