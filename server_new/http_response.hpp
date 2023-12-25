#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <string>
# include "http_request.hpp"
# include "cgi_response.hpp"

class http_response
{
private:
	int _fd;
	std::string _response;
	size_t _pos;

public:
	http_response(int fd, const http_request& hreq);
	http_response(int fd, const cgi_response& cres);

	inline bool send_all() const;
	void send_response(size_t size);

};

inline bool http_response::send_all() const
{
	return (_response.size() == _pos);
}

#endif
