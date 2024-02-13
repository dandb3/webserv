#ifndef ICYCLE_HPP
#define ICYCLE_HPP

#include <map>
#include <vector>
#include <queue>
#include "ConfigInfo.hpp"
#include "WriteFile.hpp"
#include "../modules/HttpRequest.hpp"

#define BUF_SIZE 1024UL

class ICycle
{
public:
    enum
    {
        TIMER_KEEP_ALIVE = 0,
        TIMER_REQUEST = 1
    };

protected:
    static char _buf[BUF_SIZE];

    ConfigInfo _configInfo;
    in_addr_t _localIp;
    in_port_t _localPort;
    in_addr_t _remoteIp;
    int _httpSockfd;
    int _cgiSendfd;
    int _cgiRecvfd;
    int _readFile;
    std::map<int, WriteFile> _writeFiles;
    pid_t _cgiScriptPid;
    bool _timerType; // set if it is a request timer
    bool _closed;

    std::queue<HttpRequest> _httpRequestQueue;

public:
    static char* getBuf();

    ICycle(in_addr_t localIp, in_port_t localPort, in_addr_t remoteIp, int httpSockfd);

    ConfigInfo& getConfigInfo();
    in_addr_t getLocalIp() const;
    in_port_t getLocalPort() const;
    in_addr_t getRemoteIp() const;
    int getHttpSockfd() const;
    int getCgiSendfd() const;
    int getCgiRecvfd() const;
    int getReadFile() const;
    std::map<int, WriteFile>& getWriteFiles();
    pid_t getCgiScriptPid() const;
    bool getTimerType() const;
    bool closed() const;

    void setCgiSendfd(int fd);
    void setCgiRecvfd(int fd);
    void setReadFile(int fd);
    void setCgiScriptPid(pid_t pid);
    void setTimerType(bool type);
	void setClosed();

    std::queue<HttpRequest> &getHttpRequestQueue();

};

#endif