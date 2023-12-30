#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <map>
#include <vector>
#include <sys/socket.h>
#include <sys/event.h>
#include "config_parsing/Config.hpp"

#define GETV_SIZE 10

class ServerManager
{
private:
public:
	ServerManager();						// 생성자, 만들면서 config 파일 읽어서 파싱
	ServerManager(std::string config_path); // 생성자, 만들면서 config 파일 읽어서 파싱

	void initServer(); // 서버 초기화
};

#endif
