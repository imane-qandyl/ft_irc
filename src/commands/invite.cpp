#include "../../headers/commands/invite.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"

void Invite::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // INVITE command requires exactly 2 parameters: nickname and channel
    if (params.size() < 2) {
        client.sendMessage("461 " + client.getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    std::string targetNickname = params[0];
    std::string channelName = params[1];

    // Validate nickname format
    if (!isValidNickname(targetNickname)) {
        client.sendMessage("401 " + client.getNickname() + " " + targetNickname + " :No such nick/channel\r\n");
        return;
    }

    // Validate channel name format
    if (!isValidChannelName(channelName)) {
        client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Check if channel exists
    Channel* channel = server.getChannel(channelName);
    if (!channel) {
        client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Check if inviter is in the channel
    if (!channel->hasClient(&client)) {
        client.sendMessage("442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // Check if inviter has permission to invite (must be operator if channel is invite-only)
    if (channel->isInviteOnly() && !channel->isOperator(&client)) {
        client.sendMessage("482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Find target client by nickname - use server's method directly
    Client* targetClient = server.findClientByNickname(targetNickname);
    if (!targetClient) {
        client.sendMessage("401 " + client.getNickname() + " " + targetNickname + " :No such nick/channel\r\n");
        return;
    }

    // Check if target is already in the channel
    if (channel->hasClient(targetClient)) {
        client.sendMessage("443 " + client.getNickname() + " " + targetNickname + " " + channelName + " :is already on channel\r\n");
        return;
    }

    // Add target to invite list
    channel->addInvite(targetClient);

    // Send notifications
    sendInviteNotifications(server, client, *targetClient, *channel);
}

bool Invite::isValidNickname(const std::string& nickname) {
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

bool Invite::isValidChannelName(const std::string& channelName) {
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

// REMOVE this method since we're using server.findClientByNickname() directly:
// Client* Invite::findClientByNickname(Server& server, const std::string& nickname) { ... }

void Invite::sendInviteNotifications(Server& /* server */, Client& inviter, Client& invitee, Channel& channel) {
    // Send confirmation to inviter
    inviter.sendMessage("341 " + inviter.getNickname() + " " + invitee.getNickname() + " " + channel.getName() + "\r\n");

    // Send invite notification to invitee
    invitee.sendMessage(":" + inviter.getNickname() + "!" + inviter.getUsername() + "@" + inviter.getHostname() + 
                       " INVITE " + invitee.getNickname() + " " + channel.getName() + "\r\n");

    std::cout << "[INFO] " << inviter.getNickname() << " invited " << invitee.getNickname() << " to " << channel.getName() << std::endl;
}