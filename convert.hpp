#ifndef CONVERT_HPP
# define CONVERT_HPP

# include <string>
# include "Request.hpp"
# include "Response.hpp"

void parse(const std::string& inMsg, Request& request);
void makeResponse(const Request& request, Response& response);
void makeMessage(const Response& response, std::string& outMsg);

#endif
