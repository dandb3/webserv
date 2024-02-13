#include <csignal>
#include "Cycle.hpp"

std::map<int, Cycle> Cycle::_cycleStorage;

Cycle::Cycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd)
: ICycle(localIp, localPort, remoteIp, httpSockfd)
{}

Cycle *Cycle::newCycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd)
{
    _cycleStorage.insert(std::make_pair(httpSockfd, Cycle(localIp, localPort, remoteIp, httpSockfd)));
#ifdef DEBUG
std::cout << "newCycle() called" << std::endl;
std::cout << "localIp: " << std::hex << localIp << ft_inet_ntoa(localIp) << std::endl;
std::cout << "localPort: " << localPort << std::endl;
std::cout << "remoteIp: " << std::hex << remoteIp << ft_inet_ntoa(remoteIp) << std::endl;
std::cout << "httpSockfd: " << httpSockfd << std::endl;
#endif
    return &_cycleStorage.at(httpSockfd);
}

void Cycle::deleteCycle(Cycle *cycle)
{
    _cycleStorage.erase(cycle->getHttpSockfd());
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

void Cycle::reset()
{
    _cgiSendfd = -1;
    _cgiRecvfd = -1;
    _readFile = -1;
    _writeFiles.clear();
    if (_cgiScriptPid != -1) {
        kill(_cgiScriptPid, SIGKILL);
        _cgiScriptPid = -1;
    }
    _httpResponseHandler.reset();
    _cgiRequestHandler.reset();
    _cgiResponseHandler.reset();
}
