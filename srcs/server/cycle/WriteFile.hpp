#ifndef WRITE_FILE_HPP
#define WRITE_FILE_HPP

#include <string>
#include "../../webserv.hpp"

class WriteFile
{
private:
    std::string _path;
    std::string _data;

public:
    WriteFile(const std::string& path, const std::string& data);

    int writeToFile(int fd);

    const std::string& getPath() const;

};

#endif
