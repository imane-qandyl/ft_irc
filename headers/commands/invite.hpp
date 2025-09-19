#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Invite {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidNickname(const std::string& nickname);
    static bool isValidChannelName(const std::string& channelName);
    static void sendInviteNotifications(Server& server, Client& inviter, Client& invitee, Channel& channel);
};
