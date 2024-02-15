#include "PidSet.hpp"

std::set<pid_t> PidSet::_pidSet;

void PidSet::insert(pid_t pid)
{
    _pidSet.insert(pid);
}

void PidSet::erase(pid_t pid)
{
    _pidSet.erase(pid);
}

bool PidSet::found(pid_t pid)
{
    return (_pidSet.find(pid) != _pidSet.end());
}
