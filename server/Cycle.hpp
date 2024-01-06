#ifndef CYCLE_HPP
# define CYCLE_HPP

# include <map>
# include <queue>
# include "webserv.hpp"
# include "NetConfig.hpp"

/**
 * closed 상태에서 request queue에 있는 것이 다 비어있고,
 * keep-alive가 아님 || EOF이고 (_closed 변수로 관리됨, 파싱 중에 Connection 헤더 필드를 읽거나, EOF를 감지하게 되면 set된다.),
 * HttpResponse가 전송 완료인 경우
 * cycle 객체는 할당 해제된다.
*/

class Cycle
{
private:
    static std::map<std::pair<serv_ip_t, serv_port_t>, Cycle> _cycleStorage;

    NetConfig _netConfig;
    serv_ip_t _ip;
    serv_port_t _port;
    int _httpSockfd;
	int _cgiSockfd;
    bool _closed;

    HttpRequestHandler _hrqHandler;
    HttpResponseHandler _hrspHandler;
    CgiRequestHandler _crqHandler;
    CgiResponseHandler _crspHandler;

	std::queue<HttpRequest> _hrqQueue;

public:
    static Cycle* newCycle(serv_ip_t ip, serv_port_t port, int httpSockfd);
    static void deleteCycle(Cycle* cycle);

    Cycle(serv_ip_t ip, serv_port_t port, int httpSockfd);

    inline serv_ip_t getIp() const;
    inline serv_port_t getPort() const;
    inline int getHttpSockfd() const;
    inline bool closed() const;

};

inline serv_ip_t Cycle::getIp() const
{
    return _ip;
}

inline serv_port_t Cycle::getPort() const
{
    return _port;
}

inline int Cycle::getHttpSockfd() const
{
    return _httpSockfd;
}

inline bool Cycle::closed() const
{
    return _closed;
}

#endif
