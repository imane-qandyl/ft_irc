#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include <string>
#include <vector>

class Server;
class Client;

class Pass {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidPassword(const std::string& password);
};