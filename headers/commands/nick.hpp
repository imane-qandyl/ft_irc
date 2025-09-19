#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include <string>
#include <vector>

class Server;
class Client;

class Nick {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidNickname(const std::string& nickname);
    static bool isNicknameInUse(Server& server, const std::string& nickname);
    static void sendNickChangeNotifications(Server& server, Client& client, const std::string& oldNick, const std::string& newNick);
};