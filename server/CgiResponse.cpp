#include "CgiResponseModule.hpp"

CgiResponse::CgiResponse()
{}

CgiResponse::CgiResponse(const std::string& rawCgiResponse)
// : header-fields(), message-body()
{}

CgiResponse& CgiResponse::operator=(const CgiResponse& cgiResponse)
{
    if (this == &cgiResponse)
        return *this;

    // copy header-fields
    // copy message-body
    return *this;
}
