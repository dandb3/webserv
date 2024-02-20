#include <algorithm>
#include <unistd.h>
#include "WriteFile.hpp"

WriteFile::WriteFile(const std::string& path, const std::string& data)
: _path(path), _data(data)
{}

int WriteFile::writeToFile(int fd)
{
    ssize_t writeLen;

    if ((writeLen = write(fd, _data.c_str(), _data.size())) == FAILURE)
        return FAILURE;
    return SUCCESS;
}

const std::string& WriteFile::getPath() const
{
    return _path;
}
