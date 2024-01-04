#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <map>
#include <vector>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "config_parsing/Config.hpp"
#include "KqueueHandler.hpp"

#define GETV_SIZE 10

class ServerManager
{
private:
	enum SocketType_e
	{
		SOCKET_LISTEN,
		SOCKET_CLIENT,
		SOCKET_CGI
	};
	KqueueHandler _kqueue_handler;

public:
	ServerManager();						// 생성자, 만들면서 config 파일 읽어서 파싱
	ServerManager(std::string config_path); // 생성자, 만들면서 config 파일 읽어서 파싱

	void initServer(); // 서버 초기화
};

#endif