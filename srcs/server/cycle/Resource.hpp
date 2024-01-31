#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <set>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include "Cycle.hpp"
#include "../KqueueHandler.hpp"
#include "../../webserv.hpp"

/**
 * 하나의 cycle을 기준으로 할당된 자원들을 관리하는 클래스.
 * EVFILT_PROC은 관리의 대상이 아니다.
 * 대신, 자식 프로세스가 존재하는 지 아닌지에 대한 관리만 해 준다.
*/
class Resource
{
private:
    KqueueHandler& _kqueueHandler;
    std::set<std::pair<uintptr_t, short> > _eventSet;
    std::set<int> _fdInUse;
    std::pair<bool, int> _pid;
    int _httpSockfd;

public:
    Resource(KqueueHandler& kqueueHandler);

    void eventSet(uintptr_t ident, short filter, u_short flags, u_int fflags = 0, int64_t data = 0, void* udata = NULL);

    int open(const char* path_name, int flags);
    int open(const char* path_name, int flags, mode_t mode);
    int pipe(int pipefd[2]);
    pid_t fork();
    int close(int fd);
    int kill(pid_t pid, int sig);
    pid_t waitpid(pid_t pid, int* status, int options);

    void deallocateAll();

};

#endif
