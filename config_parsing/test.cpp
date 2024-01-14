#include "Config.hpp"

int main() {
    std::cout << "======================" << std::endl;
    Config &config2 = Config::getInstance("test.txt");
    Config &config = Config::getInstance();
    std::cout << "======================" << std::endl;
    for (auto it = config.getHttpInfo().begin(); it != config.getHttpInfo().end(); it++) {
        std::cout << it->first << ": ";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            std::cout << *it2 << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "======================" << std::endl;
    std::vector<ServerConfig> &server = config.getServerConfig();
    for (auto it = server.begin(); it != server.end(); it++) {
        printParsedServer(*it);
        ServerConfig &server_config = *it;
        for (auto it2 = server_config.getServerInfo().begin(); it2 != server_config.getServerInfo().end(); it2++) {
            std::cout << it2->first << ": ";
            for (auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
                std::cout << *it3 << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "======================" << std::endl;
        std::map<std::string, LocationConfig> &location_m = it->getLocationList();
        for (auto it2 = location_m.begin(); it2 != location_m.end(); it2++) {
            std::cout << "location: " << it2->first << std::endl;
            for (auto it3 = it2->second.getLocationInfo().begin(); it3 != it2->second.getLocationInfo().end(); it3++) {
                std::cout << it3->first << ": ";
                for (auto it4 = it3->second.begin(); it4 != it3->second.end(); it4++) {
                    std::cout << *it4 << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "======================" << std::endl;
        }
    }
    return 0;
}
