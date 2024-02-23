#include <algorithm>
#include <sstream>
#include <cstring>
#include <csignal>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "PidSet.hpp"
#include "../../utils/utils.hpp"
#include "CgiRequestModule.hpp"

static std::string toProtocolSpecificKey(const std::string& key)
{
    std::string result = key;

    for (size_t i = 0; i < result.size(); ++i) {
        if (std::isalpha(result[i]))
            result[i] = std::toupper(result[i]);
        else if (result[i] == '-')
            result[i] = '_';
    }
    return result;
}

/* -------------------------- CGI request meta-variable setters -------------------------- */
static void setAuthType(CgiRequest& cgiRequest, const std::multimap<std::string, std::string>& headerFields)
{
    std::multimap<std::string, std::string>::const_iterator it;

    if ((it = headerFields.find("Authorization")) != headerFields.end())
        cgiRequest.addMetaVariable("AUTH_TYPE", it->second.substr(0, it->second.find_first_of(' ')));
}

static void setContentLength(CgiRequest& cgiRequest, const std::string& messageBody)
{
    if (!messageBody.empty())
        cgiRequest.addMetaVariable("CONTENT_LENGTH", sizeToStr(messageBody.size()));
}

static void setContentType(CgiRequest& cgiRequest, const std::multimap<std::string, std::string>& headerFields)
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

static void setGatewayInterface(CgiRequest& cgiRequest)
{
    cgiRequest.addMetaVariable("GATEWAY_INTERFACE", "CGI/1.1");
}

static void setPathInfo(CgiRequest& cgiRequest, ConfigInfo& configInfo)
{
    cgiRequest.addMetaVariable("PATH_INFO", configInfo.getPath());
}

static void setPathTranslated(CgiRequest& cgiRequest, ConfigInfo& configInfo)
{
    cgiRequest.addMetaVariable("PATH_TRANSLATED", configInfo.getPath());
}

static void setQueryString(CgiRequest& cgiRequest, const RequestLine& requestLine)
{
    const std::vector<pair_t>& query = requestLine.getQuery();
    std::string rawQuery;

    for (size_t i = 0; i < query.size(); ++i) {
        rawQuery += query[i].first + "=" + query[i].second;
        if (i + 1 != query.size())
            rawQuery += "&";
    }
    cgiRequest.addMetaVariable("QUERY_STRING", rawQuery);
}

static void setRemoteAddr(CgiRequest& cgiRequest, ICycle* cycle)
{
    cgiRequest.addMetaVariable("REMOTE_ADDR", ft_inet_ntoa(cycle->getRemoteIp()));
}

static void setRemoteHost(CgiRequest& cgiRequest, ICycle* cycle)
{
    cgiRequest.addMetaVariable("REMOTE_HOST", ft_inet_ntoa(cycle->getRemoteIp()));
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

static void setScriptName(__attribute__((unused)) CgiRequest& cgiRequest, __attribute__((unused)) ConfigInfo& configInfo)
{
    // cgiRequest.addMetaVariable("SCRIPT_NAME", configInfo.getCgiPath());
}

static void setServerName(CgiRequest& cgiRequest, ICycle* cycle)
{
    std::string serverName = cycle->getConfigInfo().getServerName();

    if (!serverName.empty())
        cgiRequest.addMetaVariable("SERVER_NAME", serverName);
    else
        cgiRequest.addMetaVariable("SERVER_NAME", ft_inet_ntoa(cycle->getLocalIp()));
}

static void setServerPort(CgiRequest& cgiRequest, ICycle* cycle)
{
    cgiRequest.addMetaVariable("SERVER_PORT", ft_itoa(static_cast<int>(cycle->getLocalPort())));
}

static void setServerProtocol(CgiRequest& cgiRequest)
{
    cgiRequest.addMetaVariable("SERVER_PROTOCOL", "HTTP/1.1");
}

static void setServerSoftware(CgiRequest& cgiRequest)
{
    cgiRequest.addMetaVariable("SERVER_SOFTWARE", "webserv/1.0");
}

static void setProtocolSpecific(CgiRequest& cgiRequest, const std::multimap<std::string, std::string>& headerFields)
{
    std::multimap<std::string, std::string>::const_iterator it;
    std::string key;

    for (it = headerFields.begin(); it != headerFields.end(); ++it) {
        key = toProtocolSpecificKey(it->first);
        if (key.size() >= 2 && key[0] == 'X' && key[1] == '_')
            cgiRequest.addMetaVariable("HTTP_" + key, it->second);
    }
}

/* -------------------- class CgiRequestHandler -------------------- */
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

void CgiRequestHandler::_setMetaVariables(ICycle* cycle, HttpRequest& httpRequest)
{
    const RequestLine& requestLine = httpRequest.getRequestLine();
    const std::string& messageBody = httpRequest.getMessageBody();
    const std::multimap<std::string, std::string>& headerFields = httpRequest.getHeaderFields();

    setAuthType(_cgiRequest, headerFields);
    
    setContentLength(_cgiRequest, messageBody);
    setContentType(_cgiRequest, headerFields);
    setGatewayInterface(_cgiRequest);
    setPathInfo(_cgiRequest, cycle->getConfigInfo());
    setPathTranslated(_cgiRequest, cycle->getConfigInfo());
    setQueryString(_cgiRequest, requestLine);
    setRemoteAddr(_cgiRequest, cycle);
    setRemoteHost(_cgiRequest, cycle);
    setRequestMethod(_cgiRequest, requestLine);
    setScriptName(_cgiRequest, cycle->getConfigInfo());
    setServerName(_cgiRequest, cycle);
    setServerPort(_cgiRequest, cycle);
    setServerProtocol(_cgiRequest);
    setServerSoftware(_cgiRequest);
    setProtocolSpecific(_cgiRequest, headerFields);
}

char** CgiRequestHandler::_makeArgv(const std::string& cgiPath, const std::string& file)
{
    char** result = new char*[3];

    result[0] = new char[cgiPath.size() + 1];
    std::strcpy(result[0], cgiPath.c_str());
    result[1] = new char[file.size() + 1];
    std::strcpy(result[1], file.c_str());
    result[2] = NULL;
    std::cout << "argv: " << result[0] << ", " << result[1] << std::endl;
    return result;
}

char** CgiRequestHandler::_makeEnvp()
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

void CgiRequestHandler::_parentProcess(int* servToCgi, int* cgiToServ)
{
    close(servToCgi[0]);
    close(cgiToServ[1]);
}

void CgiRequestHandler::_childProcess(int* servToCgi, int* cgiToServ, const std::string& cgiPath, const std::string& file)
{
    char **argv, **envp;

    dup2(servToCgi[0], STDIN_FILENO);
    dup2(cgiToServ[1], STDOUT_FILENO);
    close(servToCgi[0]);
    close(servToCgi[1]);
    close(cgiToServ[0]);
    close(cgiToServ[1]);
    argv = _makeArgv(cgiPath, file);
    envp = _makeEnvp();
    if (execve(argv[0], argv, envp) == FAILURE)
        std::exit(1);
}

/**
 * header-field find할 때, 해당 header-field 값이 여러 개가 존재할 수 있는 경우가 모두 고려된 건가?
 * 아마 안 됐을 듯.
 * 그렇다면 헤더필드에 대해서 체크 해주어야 하는 함수를 어디엔가 넣어주어야 한다..?
 * -> 파싱부에서 헤더필드를 다 읽어온 다음에 각 항목별로 Syntax Check를 해 주어야 한다.
*/

void CgiRequestHandler::makeCgiRequest(ICycle* cycle, HttpRequest& httpRequest)
{
    _setMetaVariables(cycle, httpRequest);
    _cgiRequest.setMessageBody(httpRequest.getMessageBody());
}

void CgiRequestHandler::sendCgiRequest(const struct kevent& kev)
{
    const std::string& messageBody = _cgiRequest.getMessageBody();
    ssize_t writeSize;

    if ((writeSize = write(kev.ident, messageBody.c_str() + _pos, \
        std::min(messageBody.size() - _pos, static_cast<size_t>(kev.data)))) == FAILURE)
        throw 500;
    _pos += writeSize;
    if (_pos == messageBody.size())
        _eof = true;
}

void CgiRequestHandler::callCgiScript(ICycle* cycle)
{
    int servToCgi[2], cgiToServ[2];
    pid_t pid;

    if (access(cycle->getConfigInfo().getCgiPath().c_str(), X_OK) == FAILURE)
        throw 404;
    if (pipe(servToCgi) == FAILURE)
        throw 500;
    if (pipe(cgiToServ) == FAILURE) {
        close(servToCgi[0]);
        close(servToCgi[1]);
        throw 500;
    }
    // we don't know how cgi script acts, so let I/O fds blocking state.
    fcntl(servToCgi[1], F_SETFL, O_NONBLOCK);
    fcntl(cgiToServ[0], F_SETFL, O_NONBLOCK);
    cycle->setCgiSendfd(servToCgi[1]);
    cycle->setCgiRecvfd(cgiToServ[0]);
    if ((pid = fork()) == FAILURE) {
        close(servToCgi[0]);
        close(servToCgi[1]);
        close(cgiToServ[0]);
        close(cgiToServ[1]);
        throw 500;
    }
    if (pid == 0)
        _childProcess(servToCgi, cgiToServ, cycle->getConfigInfo().getCgiPath(), cycle->getConfigInfo().getPath());
    else {
        PidSet::insert(pid);
        cycle->setCgiScriptPid(pid);
        _parentProcess(servToCgi, cgiToServ);
    }
}

bool CgiRequestHandler::eof() const
{
    return _eof;
}

void CgiRequestHandler::reset()
{
    _cgiRequest.getMetaVariables().clear();
    _cgiRequest.getMessageBody().clear();
    _pos = 0;
    _eof = false;
}
