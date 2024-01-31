#include "Resource.hpp"

Resource::Resource(KqueueHandler& kqueueHandler)
: _kqueueHandler(kqueueHandler), _fdInUse(), _pid(false, -1)
{}

void Resource::eventSet(uintptr_t ident, short filter, u_short flags, u_int fflags = 0, int64_t data = 0, void* udata = NULL)
{
    std::pair<uintptr_t, short> p(ident, filter);

    _kqueueHandler.changeEvent(ident, filter, flags, fflags, data, udata);
    if (flags & EV_ADD)
        _eventSet.insert(p);
    if (flags & EV_DELETE)
        _eventSet.erase(p);
}

int Resource::open(const char* path_name, int flags)
{
    int newFd = ::open(path_name, flags);

    if (newFd != FAILURE)
        _fdInUse.insert(newFd);
    return newFd;
}

int Resource::open(const char* path_name, int flags, mode_t mode)
{
    int newFd = ::open(path_name, flags, mode);

    if (newFd != FAILURE)
        _fdInUse.insert(newFd);
    return newFd;
}

int Resource::pipe(int pipefd[2])
{
    int res = ::pipe(pipefd);

    if (res != FAILURE) {
        _fdInUse.insert(pipefd[0]);
        _fdInUse.insert(pipefd[1]);
    }
    return res;
}

pid_t Resource::fork()
{
    pid_t pid = ::fork();

    if (pid != FAILURE) {
        _pid.first = true;
        _pid.second = pid;
    }
    return pid;
}

int Resource::close(int fd)
{
    int res = ::close(fd);

    if (res != FAILURE)
        _fdInUse.erase(fd);
    return res;
}

/* sig 값이 SIGKILL일 경우에만 호출해야 한다. */
int Resource::kill(pid_t pid, int sig)
{
    return ::kill(pid, sig);
}

pid_t Resource::waitpid(pid_t pid, int* status, int options)
{
    pid_t res = ::waitpid(pid, status, options);

    if (res != FAILURE) {
        _pid.first = false;
        _pid.second = -1;
    }
    return res;
}

void Resource::deallocateAll()
{
    std::set<int>::iterator fdIt = _fdInUse.begin();
    std::set<std::pair<uintptr_t, short> >::iterator eventIt = _eventSet.begin();

    for (; fdIt != _fdInUse.end(); ++fdIt) {
        close(*fdIt);
    }
    for (; eventIt != _eventSet.end(); ++eventIt) {
        _kqueueHandler.deleteEvent(eventIt->first, eventIt->second);
        _eventSet.erase(eventIt);
    }
    if (_pid.first == true) {
        ::kill(_pid.second, SIGKILL);
        _kqueueHandler.changeEvent(_pid.second, EVFILT_PROC, EV_ADD, NOTE_EXIT);
    }
}
