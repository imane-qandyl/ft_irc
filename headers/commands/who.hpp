#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Who {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static void handleChannelWho(Server& server, Client& client, const std::string& channelName);
    static void handleUserWho(Server& server, Client& client, const std::string& nickname);
    static void sendWhoReply(Client& client, Client* targetClient, 
                            const std::string& channel = "*");
    static void sendEndOfWho(Client& client, const std::string& target);
    static std::string getUserFlags(Client* user, Channel* channel = NULL);
};