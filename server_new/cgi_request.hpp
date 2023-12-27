#ifndef CGI_REQUEST_HPP
# define CGI_REQUEST_HPP

# include <string>
# include "http_request.hpp"

class cgi_request
{
private:
	// meta-variables

	// extension-meta-variables

public:
	cgi_request(const http_request_parser& hreq);

	// getter of meta-variables

};

#endif
