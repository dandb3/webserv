#ifndef CONVERT_HPP
# define CONVERT_HPP

# include <string>
# include "Form.hpp"

void parse(const std::string& inMsg, RequestForm& request);
void makeResponse(const RequestForm& request, ResponseForm& response);
void makeMessage(const ResponseForm& response, std::string& outMsg);

#endif
