#include "Cycle.hpp"

std::map<std::pair<serv_ip_t, serv_port_t>, Cycle> Cycle::_cycleStorage;

Cycle::Cycle(serv_ip_t ip, serv_port_t port, int httpSockfd)
: _netConfig(), _ip(ip), _port(port), _httpSockfd(httpSockfd), _closed(false)
{}

Cycle* Cycle::newCycle(serv_ip_t ip, serv_port_t port, int httpSockfd)
{
    std::pair<serv_ip_t, serv_port_t> key = std::make_pair(ip, port);

    _cycleStorage.insert(key, Cycle(ip, port, httpSockfd));
    return &_cycleStorage.at(key);
}

void Cycle::deleteCycle(Cycle* cycle)
{
    _cycleStorage.erase(std::make_pair(cycle->getIp(), cycle->getPort()));
}

serv_ip_t Cycle::getIp() const
{
    return _ip;
}

serv_port_t Cycle::getPort() const
{
    return _port;
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
    return _cgiOutputfd;
}

bool Cycle::closed() const
{
    return _closed;
}

HttpRequestHandler& Cycle::getHttpRequestHandler() const
{
    return _httpRequestHandler;
}

HttpResponseHandler& Cycle::getHttpResponseHandler() const
{
    return _httpResponseHandler;
}

CgiRequestHandler& Cycle::getCgiRequestHandler() const
{
    return _cgiRequestHandler;
}

CgiResponseHandler& Cycle::getCgiResponseHandler() const
{
    return _cgiResponseHandler;
}

std::queue<HttpRequest>& Cycle::getHttpRequestQueue() const
{
    return _httpRequestQueue;
}
