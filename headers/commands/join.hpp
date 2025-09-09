#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Join {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);
    
private:
    static bool isValidChannelName(const std::string& name);
    static void joinChannel(Server& server, Client& client, const std::string& channelName, const std::string& key = "");
    static void sendJoinMessages(Server& server, Client& client, Channel& channel);
    static void sendChannelInfo(Server& server, Client& client, Channel& channel);
};
