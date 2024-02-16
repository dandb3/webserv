#ifndef PID_SET_HPP
#define PID_SET_HPP

#include <set>
#include <unistd.h>

class PidSet
{
private:
    static std::set<pid_t> _pidSet;

public:
    static void insert(pid_t pid);
    static void erase(pid_t pid);
    static bool found(pid_t pid);

};

#endif
