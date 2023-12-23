#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <string>
# include "http_request.hpp"
# include "cgi_response.hpp"

class http_response
{
private:
	// response_line;
	// header_field;
	// message_body;

public:
	http_response(const http_request& hreq);
	http_response(const cgi_response& cres);

};

#endif
