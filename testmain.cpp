#include "config_parsing/Config.hpp"
#include "server_mingkang/ServerManager.hpp"

int main() {
    // Config::getInstance();
    ServerManager server_manager = ServerManager();
    server_manager.initServer();
    server_manager.operate();
    return 0;
}
