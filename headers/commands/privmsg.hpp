#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Privmsg {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static void sendPrivateMessage(Server& server, Client& client, const std::string& targetNick, const std::string& message);
    static void sendChannelMessage(Server& server, Client& client, const std::string& channelName, const std::string& message);
    static bool isValidNickname(const std::string& nickname);
    static bool isValidChannelName(const std::string& channelName);
    static void sendToClient(Client& sender, Client& target, const std::string& message);
    static void sendToChannel(Client& sender, Channel& channel, const std::string& message);
};