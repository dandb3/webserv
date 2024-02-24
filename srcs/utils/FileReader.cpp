#include "FileReader.hpp"

std::string FileReader::read_file(std::string filePath) {
    std::string line;
    std::string file_contents;
    std::ifstream file(filePath);
    while (std::getline(file, line)) {
        line = line.substr(0, line.find('#'));
        if (!file.eof()) {
            line += '\n';
        }
        file_contents += line;
    }
    return file_contents;
}
