#include <csignal>
#include "ICycle.hpp"

char ICycle::_buf[BUF_SIZE];

char* ICycle::getBuf()
{
    return _buf;
}

ICycle::ICycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd)
: _configInfo(), _localIp(localIp), _localPort(localPort), _remoteIp(remoteIp), _httpSockfd(httpSockfd), \
    _cgiSendfd(-1), _cgiRecvfd(-1), _readFile(-1), _writeFiles(), _cgiScriptPid(-1), _closed(false), _beDeleted(false)
{}

ConfigInfo& ICycle::getConfigInfo()
{
    return _configInfo;
}

in_addr_t ICycle::getLocalIp() const
{
    return _localIp;
}

in_port_t ICycle::getLocalPort() const
{
    return _localPort;
}

in_addr_t ICycle::getRemoteIp() const
{
    return _remoteIp;
}

int ICycle::getHttpSockfd() const
{
    return _httpSockfd;
}

int ICycle::getCgiSendfd() const
{
    return _cgiSendfd;
}

int ICycle::getCgiRecvfd() const
{
    return _cgiRecvfd;
}

int ICycle::getReadFile() const
{
    return _readFile;
}

std::map<int, WriteFile>& ICycle::getWriteFiles()
{
    return _writeFiles;
}

pid_t ICycle::getCgiScriptPid() const
{
    return _cgiScriptPid;
}

bool ICycle::closed() const
{
    return _closed;
}

bool ICycle::beDeleted() const
{
    return _beDeleted;
}

void ICycle::setHttpSockfd(int fd)
{
    _httpSockfd = fd;
}

void ICycle::setCgiSendfd(int fd)
{
    _cgiSendfd = fd;
}

void ICycle::setCgiRecvfd(int fd)
{
    _cgiRecvfd = fd;
}

void ICycle::setReadFile(int fd)
{
    _readFile = fd;
}

void ICycle::setCgiScriptPid(pid_t pid)
{
    _cgiScriptPid = pid;
}

void ICycle::setClosed()
{
    _closed = true;
}

void ICycle::setBeDeleted()
{
    _beDeleted = true;
}

std::queue<HttpRequest> &ICycle::getHttpRequestQueue()
{
    return _httpRequestQueue;
}
