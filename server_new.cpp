#include "event_handler.hpp"

void operate()
{
    event_handler handler(4);

    while (true) {
        handler.event_catch();
        for (int i = 0; i < handler.get_nevents(); ++i) {
            switch (handler.event_type(i)) {
            case event_handler::SERVER_LISTEN:
                handler.listen();
                break;
            case event_handler::HTTP_REQUEST:
                handler.http_request();
                break;
            case event_handler::HTTP_RESPONSE:
                handler.http_response();
                break;
            case event_handler::CGI_REQUEST:
                handler.cgi_request();
                break;
            case event_handler::CGI_RESPONSE:
                handler.cgi_response();
                break;
            case event_handler::SERVER_ERROR:
                handler.error();
                break;
            }
        }
    }
}
