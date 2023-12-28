#include "FileReader.hpp"

std::string FileReader::read_file(std::string file_path) {
    std::string line;
    std::string file_contents;
    std::ifstream file(file_path);
    if (file.is_open()) {
        std::cout << "File opened successfully" << std::endl;
    }
    else {
        std::cout << "File could not be opened" << std::endl;
        throw std::runtime_error("File could not be opened");
    }
    while (std::getline(file, line)) {
        line = line.substr(0, line.find('#'));
        if (!file.eof()) {
            line += '\n';
        }
        file_contents += line;
    }
    return file_contents;
}
