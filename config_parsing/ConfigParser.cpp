#include "ConfigParser.hpp"

void ConfigParser::parseConfig(std::string const& config_path, Config& config) {
    try {
        std::string file_content = FileReader::read_file(config_path);
        /* code */
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    std::cout << file_content << std::endl;
}