#ifndef CYCLE_HPP
#define CYCLE_HPP

#include <map>
#include <queue>
#include "../../webserv.hpp"
#include "ConfigInfo.hpp"
#include "../modules/HttpRequestModule.hpp"
#include "../modules/HttpResponseModule.hpp"
#include "../modules/CgiRequestModule.hpp"
#include "../modules/CgiResponseModule.hpp"

#define BUF_SIZE 1024UL

/**
 * closed 상태에서 request queue에 있는 것이 다 비어있고,
 * keep-alive가 아님 || EOF이고 (_closed 변수로 관리됨, 파싱 중에 Connection 헤더 필드를 읽거나, EOF를 감지하게 되면 set된다.),
 * HttpResponse가 전송 완료인 경우
 * cycle 객체는 할당 해제된다.
 */

class Cycle
{
public:
    enum
    {
        TIMER_KEEP_ALIVE = 0,
        TIMER_REQUEST = 1
    };

private:
    static std::map<int, Cycle> _cycleStorage;
    static char _buf[BUF_SIZE];

    ConfigInfo _configInfo;
    in_addr_t _localIp;
    in_port_t _localPort;
    in_addr_t _remoteIp;
    int _httpSockfd;
    int _cgiSendfd;
    int _cgiRecvfd;
    int _readFile;
    std::set<int> _writeFiles;
    pid_t _cgiScriptPid;
    bool _timerType; // set if it is a request timer
    bool _closed;

    HttpRequestHandler _httpRequestHandler;
    HttpResponseHandler _httpResponseHandler;
    CgiRequestHandler _cgiRequestHandler;
    CgiResponseHandler _cgiResponseHandler;

    std::queue<HttpRequest> _httpRequestQueue;

public:
    static Cycle *newCycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd);
    static void deleteCycle(Cycle *cycle);

    static char* getBuf();

    Cycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd);

    ConfigInfo& getConfigInfo();
    in_addr_t getLocalIp() const;
    in_port_t getLocalPort() const;
    in_addr_t getRemoteIp() const;
    int getHttpSockfd() const;
    int getCgiSendfd() const;
    int getCgiRecvfd() const;
    int getReadFile() const;
    std::set<int>& getWriteFiles();
    pid_t getCgiScriptPid() const;
    bool getTimerType() const;
    bool closed() const;

    HttpRequestHandler &getHttpRequestHandler();
    HttpResponseHandler &getHttpResponseHandler();
    CgiRequestHandler &getCgiRequestHandler();
    CgiResponseHandler &getCgiResponseHandler();
    std::queue<HttpRequest> &getHttpRequestQueue();

    void setCgiSendfd(int fd);
    void setCgiRecvfd(int fd);
    void setReadFile(int fd);
    void setCgiScriptPid(pid_t pid);
    void setTimerType(bool type);
	void setClosed();

    void resetCycle();
};

#endif
