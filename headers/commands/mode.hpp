#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Mode {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params);

private:
    static bool isValidChannelName(const std::string& channelName);
    static void processChannelModes(Server& server, Client& client, Channel& channel, 
                                   const std::string& modeString, const std::vector<std::string>& args);
    static void handleInviteOnly(Client& client, Channel& channel, bool add, std::string& response);
    static void handleTopicRestriction(Client& client, Channel& channel, bool add, std::string& response);
    static void handleChannelKey(Client& client, Channel& channel, bool add, const std::string& key, std::string& response);
    static void handleOperatorPrivilege(Server& server, Client& client, Channel& channel, bool add, const std::string& nickname, std::string& response);
    static void handleUserLimit(Client& client, Channel& channel, bool add, const std::string& limit, std::string& response);
    static void sendModeResponse(Client& client, Channel& channel, const std::string& modeChanges);
    static bool isValidNickname(const std::string& nickname);
    static bool isNumeric(const std::string& str);
};

