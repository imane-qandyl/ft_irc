#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Kick {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidChannelName(const std::string& channelName);
    static bool isValidNickname(const std::string& nickname);
    static void kickClient(Server& server, Client& kicker, Client& target, Channel& channel, const std::string& reason);
    static void sendKickMessages(Server& server, Client& kicker, Client& target, Channel& channel, const std::string& reason);
};