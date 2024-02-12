#ifndef CYCLE_HPP
#define CYCLE_HPP

#include <map>
#include <queue>
#include "ICycle.hpp"
#include "ConfigInfo.hpp"
#include "WriteFile.hpp"
#include "../modules/HttpRequestModule.hpp"
#include "../modules/HttpResponseModule.hpp"
#include "../modules/CgiRequestModule.hpp"
#include "../modules/CgiResponseModule.hpp"

/**
 * closed 상태에서 request queue에 있는 것이 다 비어있고,
 * keep-alive가 아님 || EOF이고 (_closed 변수로 관리됨, 파싱 중에 Connection 헤더 필드를 읽거나, EOF를 감지하게 되면 set된다.),
 * HttpResponse가 전송 완료인 경우
 * cycle 객체는 할당 해제된다.
 */
class Cycle : public ICycle
{
private:
    static std::map<int, Cycle> _cycleStorage;

    HttpRequestHandler _httpRequestHandler;
    HttpResponseHandler _httpResponseHandler;
    CgiRequestHandler _cgiRequestHandler;
    CgiResponseHandler _cgiResponseHandler;

public:
    static Cycle *newCycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd);
    static void deleteCycle(Cycle *cycle);

    Cycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd);

    HttpRequestHandler &getHttpRequestHandler();
    HttpResponseHandler &getHttpResponseHandler();
    CgiRequestHandler &getCgiRequestHandler();
    CgiResponseHandler &getCgiResponseHandler();
    

    void reset();
};

#endif
