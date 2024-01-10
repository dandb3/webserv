#ifndef CYCLE_HPP
# define CYCLE_HPP

# include <map>
# include <queue>
# include "webserv.hpp"
# include "NetConfig.hpp"
# include "HttpRequestModule.hpp"
# include "HttpResponseModule.hpp"
# include "CgiRequestModule.hpp"
# include "CgiResponseModule.hpp"

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
	int _cgiSendfd;
    int _cgiRecvfd;
    bool _closed;

    HttpRequestHandler _httpRequestHandler;
    HttpResponseHandler _httpResponseHandler;
    CgiRequestHandler _cgiRequestHandler;
    CgiResponseHandler _cgiResponseHandler;

	std::queue<HttpRequest> _httpRequestQueue;

public:
    static Cycle* newCycle(serv_ip_t ip, serv_port_t port, int httpSockfd);
    static void deleteCycle(Cycle* cycle);

    Cycle(serv_ip_t ip, serv_port_t port, int httpSockfd);

    serv_ip_t getIp() const;
    serv_port_t getPort() const;
    int getHttpSockfd() const;
    int getCgiSendfd() const;
    int getCgiRecvfd() const;
    bool closed() const;

    HttpRequestHandler& getHttpRequestHandler() const;
    HttpResponseHandler& getHttpResponseHandler() const;
    CgiRequestHandler& getCgiRequestHandler() const;
    CgiResponseHandler& getCgiResponseHandler() const;
    std::queue<HttpRequest>& getHttpRequestQueue() const;

    void resetCycle();

};

#endif
