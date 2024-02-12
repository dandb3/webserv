#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <iostream>
#include <string>
#include <fstream>

class FileReader
{
public:
    static std::string read_file(std::string filePath);
};

#endif