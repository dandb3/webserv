#include <algorithm>
#include <unistd.h>
#include "WriteFile.hpp"

WriteFile::WriteFile(const std::string& path, const std::string& data)
: _path(path), _data(data), _pos(0), _eof(false)
{}

int WriteFile::writeToFile(int fd, size_t size)
{
    ssize_t writeLen;

    if ((writeLen = write(fd, _data.c_str() + _pos, std::min(_data.size() - _pos, size))) == FAILURE)
        return FAILURE;
    _pos += writeLen;
    if (_pos == _data.size())
        _eof = true;
    return SUCCESS;
}

const std::string& WriteFile::getPath() const
{
    return _path;
}

bool WriteFile::eof() const
{
    return _eof;
}
