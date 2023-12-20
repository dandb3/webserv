#include "utils/FileReader.hpp"

int main() {
    std::cout << FileReader::read_file("config_parsing/test.txt") << std::endl;
    return 0;
}