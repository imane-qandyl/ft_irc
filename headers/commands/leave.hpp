#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>
#include <sstream>

class Server;
class Client;
class Channel;

class Leave {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidChannelName(const std::string& channelName);
    static void leaveChannel(Server& server, Client& client, const std::string& channelName, const std::string& reason);
    static void sendPartMessages(Server& server, Client& client, Channel& channel, const std::string& reason);
};
