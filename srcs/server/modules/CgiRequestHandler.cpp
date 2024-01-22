#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "../../utils/utils.hpp"
#include "HttpRequestModule.hpp"
#include "CgiRequestModule.hpp"

/* 제거 예정 */
#ifndef HTTP_REQUEST_MODULE_HPP
#include <map>
#endif

/* -------------------------- CGI request meta-variable setters -------------------------- */
static void setAuthType(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setContentLength(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setContentType(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setGatewayInterface(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setPathInfo(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setPathTranslated(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setQueryString(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setRemoteAddr(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setRemoteHost(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setRequestMethod(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setScriptName(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setServerName(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setServerPort(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setServerProtocol(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setServerSoftware(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setProtocolSpecific(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);

CgiRequestHandler::CgiRequestHandler()
: _cgiRequest(), _pos(0), _eof(false)
{}

CgiRequestHandler& CgiRequestHandler::operator=(const CgiRequestHandler& cgiRequestHandler)
{
    if (this == &cgiRequestHandler)
        return *this;

    _cgiRequest = cgiRequestHandler._cgiRequest;
    _pos = cgiRequestHandler._pos;
    _eof = cgiRequestHandler._eof;
    return *this;
}

void CgiRequestHandler::_setMetaVariables(Cycle* cycle, HttpRequest& httpRequest)
{
    const RequestLine& requestLine = httpRequest.getRequestLine();
    const std::string& messageBody = httpRequest.getMessageBody();
    const std::multimap<std::string, std::string>& headerFields = httpRequest.getHeaderFields();

    setAuthType(_cgiRequest, requestLine, headerFields, messageBody);
    setContentLength(_cgiRequest, requestLine, headerFields, messageBody);
    setContentType(_cgiRequest, requestLine, headerFields, messageBody);
    setGatewayInterface(_cgiRequest, requestLine, headerFields, messageBody);
    setPathInfo(_cgiRequest, requestLine, headerFields, messageBody);
    setPathTranslated(_cgiRequest, requestLine, headerFields, messageBody);
    setQueryString(_cgiRequest, requestLine, headerFields, messageBody);
    setRemoteAddr(_cgiRequest, requestLine, headerFields, messageBody);
    setRemoteHost(_cgiRequest, requestLine, headerFields, messageBody);
    setRequestMethod(_cgiRequest, requestLine, headerFields, messageBody);
    setScriptName(_cgiRequest, requestLine, headerFields, messageBody);
    setServerName(_cgiRequest, requestLine, headerFields, messageBody);
    setServerPort(_cgiRequest, requestLine, headerFields, messageBody);
    setServerProtocol(_cgiRequest, requestLine, headerFields, messageBody);
    setServerSoftware(_cgiRequest, requestLine, headerFields, messageBody);
    setProtocolSpecific(_cgiRequest, requestLine, headerFields, messageBody);
}

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
    const std::vector<std::string>& metaVariables = _cgiRequest.getMetaVariables();
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
    char **argv, **envp;

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

/**
 * header-field find할 때, 해당 header-field 값이 여러 개가 존재할 수 있는 경우가 모두 고려된 건가?
 * 아마 안 됐을 듯.
 * 그렇다면 헤더필드에 대해서 체크 해주어야 하는 함수를 어디엔가 넣어주어야 한다..?
 * -> 파싱부에서 헤더필드를 다 읽어온 다음에 각 항목별로 Syntax Check를 해 주어야 한다.
*/

void CgiRequestHandler::makeCgiRequest(Cycle* cycle, HttpRequest& httpRequest)
{
    _setMetaVariables(cycle, httpRequest);
    _cgiRequest.setMessageBody(httpRequest.getMessageBody());
}

void CgiRequestHandler::sendCgiRequest(const struct kevent& kev)
{
    const std::string& messageBody = _cgiRequest.getMessageBody();
    size_t remainSize, sendSize, maxSize = static_cast<size_t>(kev.data);

    remainSize = messageBody.size() - _pos;
    sendSize = (remainSize < maxSize) ? remainSize : maxSize;

    if (write(kev.ident, messageBody.c_str() + _pos, sendSize) == FAILURE)
        throw ERROR;
    _pos += sendSize;
    if (remainSize <= maxSize)
        _eof = true;
}

void CgiRequestHandler::callCgiScript(int& cgiSendFd, int& cgiRecvFd) const
{
    int servToCgi[2], cgiToServ[2];
    pid_t pid;

    if (pipe(servToCgi) == FAILURE || pipe(cgiToServ) == FAILURE)
        throw ERROR;
    // we don't know how cgi script acts, so let I/O fds blocking state.
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

bool CgiRequestHandler::eof() const
{
    return _eof;
}

/* ----------- static functions ----------- */

static void setAuthType(CgiRequest& cgiRequest, __attribute__((unused)) const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, __attribute__((unused)) const std::string& messageBody)
{
    std::multimap<std::string, std::string>::const_iterator it;

    if ((it = headerFields.find("Authorization")) != headerFields.end())
        cgiRequest.addMetaVariable("AUTH_TYPE", it->second.substr(0, it->second.find_first_of(' ')));
}

static void setContentLength(CgiRequest& cgiRequest, __attribute__((unused)) const RequestLine& requestLine, __attribute__((unused)) const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody)
{
    if (!messageBody.empty())
        cgiRequest.addMetaVariable("CONTENT_LENGTH", sizeToStr(messageBody.size()));
}

static void setContentType(CgiRequest& cgiRequest, __attribute__((unused)) const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, __attribute__((unused)) const std::string& messageBody)
{
    std::multimap<std::string, std::string>::const_iterator it;

    if ((it = headerFields.find("Content-Type")) != headerFields.end())
        cgiRequest.addMetaVariable("CONTENT_TYPE", it->second);
    /**
     * 사실은 message-body를 통해 content-type을 추론하는 과정이 필요한데
     * 편의상 생략했고, 추후 추가 가능.
    */
    else
        cgiRequest.addMetaVariable("CONTENT_TYPE", "application/octet-stream");
}

static void setGatewayInterface(CgiRequest& cgiRequest, __attribute__((unused)) const RequestLine& requestLine, __attribute__((unused)) const std::multimap<std::string, std::string>& headerFields, __attribute__((unused)) const std::string& messageBody)
{
    cgiRequest.addMetaVariable("GATEWAY_INTERFACE", "CGI/1.1");
}

static void setPathInfo(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody)
{
    /**
     * hmm.. ㄴㅏ머지 코드를 알아야 할 듯.
    */
}

static void setPathTranslated(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setQueryString(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody);
static void setRemoteAddr(CgiRequest& cgiRequest, in_addr_t addr)
{
    cgiRequest.addMetaVariable("REMOTE_ADDR", ft_inet_ntoa(addr));
}

static void setRemoteHost(CgiRequest& cgiRequest, in_addr_t addr)
{
    cgiRequest.addMetaVariable("REMOTE_HOST", ft_inet_ntoa(addr));
}

static void setRequestMethod(CgiRequest& cgiRequest, const RequestLine& requestLine)
{
    switch (requestLine.getMethod()) {
    case HttpRequestHandler::GET:
        cgiRequest.addMetaVariable("REQUEST_METHOD", "GET");
        break;
    case HttpRequestHandler::HEAD:
        cgiRequest.addMetaVariable("REQUEST_METHOD", "HEAD");
        break;
    case HttpRequestHandler::POST:
        cgiRequest.addMetaVariable("REQUEST_METHOD", "POST");
        break;
    case HttpRequestHandler::DELETE:
        cgiRequest.addMetaVariable("REQUEST_METHOD", "DELETE");
        break;
    }
}

static void setScriptName(CgiRequest& cgiRequest)
{
    cgiRequest.addMetaVariable("SCRIPT_NAME", CGI_PATH);
}

static void setServerName(CgiRequest& cgiRequest, Cycle* cycle)
{
    if (cycle->getConfigInfo())
}

static void setServerPort(CgiRequest& cgiRequest, Cycle* cycle)
{
    cgiRequest.addMetaVariable("SERVER_PORT", ft_itoa(static_cast<int>(cycle->getPort())));
}

static void setServerProtocol(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody)
{
    cgiRequest.addMetaVariable("SERVER_PROTOCOL", "HTTP/1.1");
}

static void setServerSoftware(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody)
{
    cgiRequest.addMetaVariable("SERVER_SOFTWARE", "webserv/1.0");
}

static void setProtocolSpecific(CgiRequest& cgiRequest, const RequestLine& requestLine, const std::multimap<std::string, std::string>& headerFields, const std::string& messageBody)
{
    /**
     * 앞서 설정하고 남은 헤더필드의 경우, 여러 예외사항 (노션에 업로드 예정) 을 제외하고,
     * HTTP_ 접두사를 붙여서 그대로 CGI Script에 전달한다.
    */
}
