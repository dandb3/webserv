#include "Cycle.hpp"

std::map<int, Cycle> Cycle::_cycleStorage;

Cycle::Cycle(serv_ip_t ip, serv_port_t port, int httpSockfd)
    : _configInfo(), _ip(ip), _port(port), _httpSockfd(httpSockfd), _closed(false)
{}

Cycle *Cycle::newCycle(serv_ip_t ip, serv_port_t port, int httpSockfd)
{
    _cycleStorage.insert(std::make_pair(httpSockfd, Cycle(ip, port, httpSockfd)));
    return &_cycleStorage.at(httpSockfd);
}

void Cycle::deleteCycle(Cycle *cycle)
{
    _cycleStorage.erase(cycle->getHttpSockfd());
}

const ConfigInfo& Cycle::getConfigInfo() const
{
    return _configInfo;
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
    return _cgiRecvfd;
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

std::queue<HttpRequest> &Cycle::getHttpRequestQueue()
{
    return _httpRequestQueue;
}

void Cycle::resetCycle()
{
    _configInfo = ConfigInfo(); // 맞나?
    // _httpRequestHandler = HttpRequestHandler();
    // _httpResponseHandler = HttpResponseHandler();
    _cgiRequestHandler = CgiRequestHandler();
    _cgiResponseHandler = CgiResponseHandler();
}
