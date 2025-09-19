#pragma once


#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Topic {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidChannelName(const std::string& channelName);
    static void showTopic(Client& client, Channel& channel);
    static void setTopic(Client& client, Channel& channel, const std::string& topic);
    static void broadcastTopicChange(Client& client, Channel& channel, const std::string& topic);
};
