#include "Cycle.hpp"

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
