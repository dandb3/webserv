#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <map>
#include <vector>
#include <set>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include "../config/Config.hpp"
#include "EventHandler.hpp"

#define GETV_SIZE 10

class ServerManager
{
private:
	EventHandler _eventHandler;
	enum SocketType_e
	{
		SOCKET_LISTEN,
		SOCKET_CLIENT,
		SOCKET_CGI
	};

public:
	ServerManager();						// 생성자, 만들면서 config 파일 읽어서 파싱
	ServerManager(std::string config_path); // 생성자, 만들면서 config 파일 읽어서 파싱

	void initServer(); // 서버 초기화
	void operate();	   // 서버 동작
};

#endif
