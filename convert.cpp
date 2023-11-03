#include "convert.hpp"

void parse(const std::string& inMsg, RequestForm& request)
{
    std::string stLine, hdrField, msgBody;
    request.insertStart();
    request.insertHeader();
    request.insertBody();
}

void makeResponse(const RequestForm& request, ResponseForm& response)
{}

void makeMessage(const ResponseForm& response, std::string& outMsg)
{}
