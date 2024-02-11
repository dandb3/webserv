#include "webserv.hpp"

int main(int argc, char* argv[])
{
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << "[config file]" << std::endl;
        std::cerr << "    or " << argv[0] << std::endl;
        return 1;
    }

    if (argc == 1) {
        ServerManager serverManager;

        serverManager.operate();
    }
    else {
        ServerManager serverManager(argv[1]);

        serverManager.operate();
    }
    return 0;
}
