#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <map>
# include <vector>
# include <sys/socket.h>
# include <sys/event.h>
# include "Config.hpp"
# include "event_handler.hpp"
# include "http_request_parser.hpp"
# include "http_request.hpp"
# include "http_response.hpp"
# include "cgi_request.hpp"
# include "cgi_response.hpp"
# include "webserv.hpp"

# define GETV_SIZE 10

class server_manager
{
private:
    enum socket_type
    {
		SOCKET_LISTEN,
		SOCKET_HTTP,
		SOCKET_CGI,
    };

	enum event_type
	{
		EVENT_LISTEN,
		EVENT_HTTP_REQ,
		EVENT_HTTP_RES,
		EVENT_CGI_REQ,
		EVENT_CGI_RES,
		EVENT_ERROR,
	};

    const Config& _conf;
    std::map<int, char> _type_m;
    std::map<int, http_request_parser> _http_request_m;
	std::map<int, http_response> _http_response_m;
	std::map<int, cgi_request> _cgi_request_m;
	std::map<int, cgi_response> _cgi_response_m;
    event_handler _handler;

//  만들어진 http_request를 cgi_request 혹은 http_response로 만들어주는 함수.
    void _handle_http_request(const http_request& hreq, http_response& hres);

//  get type corresponding to fd value
    int _get_type(const struct kevent& kev);

//  main functions of this class
    void _serv_listen(const struct kevent& kev);
    void _serv_http_request(const struct kevent& kev);
    void _serv_http_response(const struct kevent& kev);
    void _serv_cgi_request(const struct kevent& kev);
    void _serv_cgi_response(const struct kevent& kev);
    void _serv_error(const struct kevent& kev);

public:
    server_manager();

    void operate();

};

#endif