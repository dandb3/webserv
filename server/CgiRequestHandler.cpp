#include <cstring>
#include <algorithm>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "CgiRequestModule.hpp"

CgiRequestHandler::CgiRequestHandler()
: _cgiRequest(), _pos(0)
{}

char** CgiRequestHandler::_makeArgv() const
{
    char** result = new char*[2];

    result[0] = new char[std::strlen(CGI_PATH) + 1];
    std::strcpy(result[0], CGI_PATH);
    result[1] = NULL;
    return result;
}

char** CgiRequestHandler::_makeEnvp() const
{
    std::vector<std::string>& metaVariables = _cgiRequest.getMetaVariables();
    char** result = new char*[metaVariables.size() + 1];

    for (size_t i = 0; i < metaVariables.size(); ++i) {
        result[i] = new char[metaVariables[i].size() + 1];
        std::strcpy(result[i], metaVariables[i].c_str());
    }
    result[metaVariables.size()] = NULL;

    return result;
}

void CgiRequestHandler::_parentProcess(int* servToCgi, int* cgiToServ) const
{
    if (close(servToCgi[0]) == FAILURE || close(cgiToServ[1]) == FAILURE)
        throw ERROR;
}

void CgiRequestHandler::_childProcess(int* servToCgi, int* cgiToServ) const
{
    char** argv, envp;

    if (dup2(servToCgi[0], STDIN_FILENO) == FAILURE \
        || dup2(cgiToServ[1], STDOUT_FILENO) == FAILURE)
        exit(1);
    if (close(servToCgi[0]) == FAILURE || close(servToCgi[1]) == FAILURE \
        || close(cgiToServ[0]) == FAILURE || close(cgiToServ[1]) == FAILURE)
        exit(1);
    argv = _makeArgv();
    envp = _makeEnvp();
    if (execve(argv[0], argv, envp) == FAILURE)
        exit(1);
}

void CgiRequestHandler::callCgiScript(int& cgiSendFd, int& cgiRecvFd) const
{
    int servToCgi[2], cgiToServ[2];
    pid_t pid;

    if (pipe(servToCgi) == FAILURE || pipe(cgiToServ) == FAILURE)
        throw ERROR;
    // we don't know how cgi script acts, so let them blocking fds.
    if (fcntl(servToCgi[1], F_SETFL, O_NONBLOCK) == FAILURE \
        || fcntl(cgiToServ[0], F_SETFL, O_NONBLOCK) == FAILURE)
        throw ERROR;
    cgiSendFd = servToCgi[1];
    cgiRecvFd = cgiToServ[0];
    if ((pid = fork()) == FAILURE)
        throw ERROR;
    if (pid == 0)
        _childProcess(servToCgi, cgiToServ);
    else
        _parentProcess(servToCgi, cgiToServ);
}

void CgiRequestHandler::sendCgiRequest(int fd, size_t size)
{
    const std::string& messageBody = _cgiRequst.getMessageBody();
    size_t remainSize, sendSize;

    remainSize = messageBody.size() - _pos;
    sendSize = std::min(remainSize, size);

    if (write(fd, messageBody.c_str() + _pos, sendSize) == FAILURE)
        throw ERROR;
    _pos += sendSize;
    if (remainSize <= size)
        close(fd); // no need to call deleteEvent();
}
