#include "convert.hpp"

void parse(const std::string& inMsg, Request& request)
{
    std::string stLine, hdrField, msgBody;
    request.insertStart();
    request.insertHeader();
    request.insertBody();
}

void makeResponse(const Request& request, Response& response)
{}

void makeMessage(const Response& response, std::string& outMsg)
{}
