#include "../../headers/commands/kick.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"

void Kick::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // KICK command requires at least 2 parameters: channel and nickname
    if (params.size() < 2) {
        client.sendMessage("461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
        return;
    }

    std::string channelName = params[0];
    std::string targetNickname = params[1];
    std::string reason = params.size() > 2 ? params[2] : client.getNickname();

    // Validate channel name format
    if (!isValidChannelName(channelName)) {
        client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Validate target nickname format
    if (!isValidNickname(targetNickname)) {
        client.sendMessage("401 " + client.getNickname() + " " + targetNickname + " :No such nick/channel\r\n");
        return;
    }

    // Check if channel exists
    Channel* channel = server.getChannel(channelName);
    if (!channel) {
        client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Check if kicker is in the channel
    if (!channel->hasClient(&client)) {
        client.sendMessage("442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // Check if kicker is a channel operator
    if (!channel->isOperator(&client)) {
        client.sendMessage("482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Find target client by nickname
    Client* targetClient = server.findClientByNickname(targetNickname);
    if (!targetClient) {
        client.sendMessage("401 " + client.getNickname() + " " + targetNickname + " :No such nick/channel\r\n");
        return;
    }

    // Check if target is in the channel
    if (!channel->hasClient(targetClient)) {
        client.sendMessage("441 " + client.getNickname() + " " + targetNickname + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    // Can't kick yourself
    if (targetClient == &client) {
        client.sendMessage("484 " + client.getNickname() + " " + channelName + " :Cannot kick yourself\r\n");
        return;
    }

    // Perform the kick
    kickClient(server, client, *targetClient, *channel, reason);
}

bool Kick::isValidChannelName(const std::string& channelName) {
    if (channelName.empty() || channelName.length() > 50) {
        return false;
    }

    // Channel names must start with # or &
    if (channelName[0] != '#' && channelName[0] != '&') {
        return false;
    }

    // Check for invalid characters
    for (size_t i = 0; i < channelName.length(); ++i) {
        char c = channelName[i];
        if (c == ' ' || c == ',' || c == '\007') {
            return false;
        }
    }

    return true;
}

bool Kick::isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }

    // Check for valid IRC nickname characters
    for (size_t i = 0; i < nickname.length(); ++i) {
        char c = nickname[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == '_' || c == '-' || 
              c == '[' || c == ']' || c == '\\' || c == '`' || 
              c == '^' || c == '{' || c == '}')) {
            return false;
        }
    }

    return true;
}

void Kick::kickClient(Server& server, Client& kicker, Client& target, Channel& channel, const std::string& reason) {
    // Send KICK messages before removing the client
    sendKickMessages(server, kicker, target, channel, reason);

    // Remove target from channel
    channel.removeClient(&target);
    target.removeChannel(&channel);

    std::cout << "[INFO] " << kicker.getNickname() << " kicked " << target.getNickname() 
              << " from " << channel.getName() << " (reason: " << reason << ")" << std::endl;

    // Remove channel if it's empty
    if (channel.getClientCount() == 0) {
        server.removeChannel(channel.getName());
        std::cout << "[INFO] Channel " << channel.getName() << " removed (empty)" << std::endl;
    }
}

void Kick::sendKickMessages(Server& /* server */, Client& kicker, Client& target, Channel& channel, const std::string& reason) {
    std::string kickMsg = ":" + kicker.getNickname() + "!" + kicker.getUsername() + "@" + kicker.getHostname() + 
                         " KICK " + channel.getName() + " " + target.getNickname() + " :" + reason + "\r\n";

    // Send KICK message to all clients in the channel (including the kicked client)
    channel.broadcast(kickMsg);
}