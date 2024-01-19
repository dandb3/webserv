#include "srcs/config/Config.hpp"
#include "srcs/server/ServerManager.hpp"

int main() {
    // Config::getInstance();
    ServerManager server_manager = ServerManager();
    server_manager.initServer();
    server_manager.operate();
    return 0;
}
