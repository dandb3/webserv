#include <csignal>
#include "Cycle.hpp"
#include "../modules/PidSet.hpp"

std::map<int, Cycle> Cycle::_cycleStorage;

Cycle::Cycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd)
: ICycle(localIp, localPort, remoteIp, httpSockfd)
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
    _httpRequestHandler.reset();
    _httpResponseHandler.reset();
    _cgiRequestHandler.reset();
    _cgiResponseHandler.reset();
}
