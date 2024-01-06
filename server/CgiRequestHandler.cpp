#include <cstring>
#include <unistd.h>
#include "CgiRequestModule.hpp"

CgiRequestHandler::CgiRequestHandler()
: _cgiRequest(), _pos(0)
{}

char** CgiRequestHandler::_makeEnvp() const
{
    std::vector<std::string>& metaVariables = _cgiRequest._metaVariables;
    char** result = new char*[metaVariables.size() + 1];

    for (size_t i = 0; i < metaVariables.size(); ++i) {
        result[i] = new char[metaVariables[i].size() + 1];
        std::strcpy(result[i], metaVariables[i].c_str());
    }
    result[metaVariables.size()] = NULL;

    return result;
}

void CgiRequestHandler::sendCgiRequest(int sockfd, size_t size)
{

}
