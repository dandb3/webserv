#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <string>
# include "http_request_parser.hpp"
# include "cgi_response.hpp"

class http_response
{
public:
	enum
	{
		RES_IDLE,
		RES_PROCESSING,
		RES_READY,
	};

private:
	int _fd;
	std::string _response;
	size_t _pos;
	char _status;

public:
	http_response(int fd, const http_request_parser& hreq);
	http_response(int fd, const cgi_response& cres);

	inline int get_status() const;
	inline void set_status(int status);

	void send_response(size_t size);

};

inline int http_response::get_status() const
{
	return _status;
}

inline void http_response::set_status(int status)
{
	_status = static_cast<char>(status);
}

#endif
