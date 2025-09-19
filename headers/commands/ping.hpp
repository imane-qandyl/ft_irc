#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include <string>
#include <vector>

class Server;
class Client;

class Ping {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidOrigin(const std::string& origin);
};