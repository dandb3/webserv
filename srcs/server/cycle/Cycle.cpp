#include "Cycle.hpp"

std::map<int, Cycle> Cycle::_cycleStorage;

Cycle::Cycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd)
: _configInfo(), _localIp(localIp), _localPort(localPort), _remoteIp(remoteIp), _httpSockfd(httpSockfd), _isRTimer(true), _closed(false)
{}

Cycle *Cycle::newCycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd)
{
    _cycleStorage.insert(std::make_pair(httpSockfd, Cycle(localIp, localPort, remoteIp, httpSockfd)));
    return &_cycleStorage.at(httpSockfd);
}

void Cycle::deleteCycle(Cycle *cycle)
{
    _cycleStorage.erase(cycle->getHttpSockfd());
}

ConfigInfo& Cycle::getConfigInfo()
{
    return _configInfo;
}

in_addr_t Cycle::getLocalIp() const
{
    return _localIp;
}

in_port_t Cycle::getLocalPort() const
{
    return _localPort;
}

in_addr_t Cycle::getRemoteIp() const
{
    return _remoteIp;
}

int Cycle::getHttpSockfd() const
{
    return _httpSockfd;
}

int Cycle::getCgiSendfd() const
{
    return _cgiSendfd;
}

int Cycle::getCgiRecvfd() const
{
    return _cgiRecvfd;
}

pid_t Cycle::getCgiScriptPid() const
{
    return _cgiScriptPid;
}

bool Cycle::isRTimer() const
{
    return _isRTimer;
}

bool Cycle::closed() const
{
    return _closed;
}

HttpRequestHandler &Cycle::getHttpRequestHandler()
{
    return _httpRequestHandler;
}

HttpResponseHandler &Cycle::getHttpResponseHandler()
{
    return _httpResponseHandler;
}

CgiRequestHandler &Cycle::getCgiRequestHandler()
{
    return _cgiRequestHandler;
}

CgiResponseHandler &Cycle::getCgiResponseHandler()
{
    return _cgiResponseHandler;
}

HttpRequestQueue &Cycle::getHttpRequestQueue()
{
    return _httpRequestQueue;
}

void Cycle::setCgiSendfd(int fd)
{
    _cgiSendfd = fd;
}

void Cycle::setCgiRecvfd(int fd)
{
    _cgiRecvfd = fd;
}

void Cycle::setCgiScriptPid(pid_t pid)
{
    _cgiScriptPid = pid;
}

void Cycle::resetCycle()
{
    // _httpRequestHandler = HttpRequestHandler();
    // _httpResponseHandler = HttpResponseHandler();
    _cgiRequestHandler = CgiRequestHandler();
    _cgiResponseHandler = CgiResponseHandler();
}
