#include "webserv.hpp"
#include "server/ServerManager.hpp"

int main(int argc, char* argv[])
{
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << "[config file]" << std::endl;
        std::cerr << "    or " << argv[0] << std::endl;
        return 1;
    }

    ServerManager *serverManager;

    try {
        if (argc == 1)
            serverManager = new ServerManager();
        else
            serverManager = new ServerManager(argv[1]);

        if (serverManager == NULL)
            throw std::runtime_error("ServerManager creation failed");
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    serverManager->operate();

    delete serverManager;
    return 0;
}
