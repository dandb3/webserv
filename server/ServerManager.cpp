#include <iostream>

#include "ServerManager.hpp"

ServerManager::ServerManager()
{
    Config& config = Config::getInstance();
}

ServerManager::ServerManager(std::string config_path)
{
    Config& config = Config::getInstance(config_path);
}

void ServerManager::initServer()
{
    Config& config = Config::getInstance();
}
