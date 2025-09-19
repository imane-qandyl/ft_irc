#include "../../headers/commands/privmsg.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"

#include "../../headers/commands/privmsg.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"

void Privmsg::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // PRIVMSG command requires at least 2 parameters: target and message
    if (params.size() < 2) {
        client.sendMessage("461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
        return;
    }

    std::string target = params[0];
    std::string message = params[1];

    // Check if target is a channel or a user
    if (target[0] == '#' || target[0] == '&') {
        // Channel message
        sendChannelMessage(server, client, target, message);
    } else {
        // Private message to user
        sendPrivateMessage(server, client, target, message);
    }
}

void Privmsg::sendPrivateMessage(Server& server, Client& client, const std::string& targetNick, const std::string& message) {
    // Find target client
    Client* targetClient = server.findClientByNickname(targetNick);
    if (!targetClient) {
        client.sendMessage("401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

    // Create PRIVMSG in IRC format
    std::string privmsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
                         " PRIVMSG " + targetNick + " :" + message + "\r\n";

    // Send message to target client
    targetClient->sendMessage(privmsg);
    
    std::cout << "[INFO] Private message sent from " << client.getNickname() << " to " << targetNick << std::endl;
}

void Privmsg::sendChannelMessage(Server& server, Client& client, const std::string& channelName, const std::string& message) {
    // Get channel
    Channel* channel = server.getChannel(channelName);
    if (!channel) {
        client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Check if client is in the channel
    if (!channel->hasClient(&client)) {
        client.sendMessage("404 " + client.getNickname() + " " + channelName + " :Cannot send to channel\r\n");
        return;
    }

    // Create PRIVMSG in IRC format
    std::string channelMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
                            " PRIVMSG " + channelName + " :" + message + "\r\n";

    // Send message to all clients in channel except sender
    channel->broadcast(channelMsg, &client);
    
    std::cout << "[INFO] Channel message sent from " << client.getNickname() << " to " << channelName << std::endl;
}

bool Privmsg::isValidNickname(const std::string& nickname) {
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

bool Privmsg::isValidChannelName(const std::string& channelName) {
    if (channelName.empty() || channelName.length() > 50) {
        return false;
    }

    // Channel names must start with # or &
    return (channelName[0] == '#' || channelName[0] == '&');
}

void Privmsg::sendToClient(Client& sender, Client& target, const std::string& message) {
    std::string privmsgFormat = ":" + sender.getNickname() + "!" + sender.getUsername() + "@" + sender.getHostname() + 
                               " PRIVMSG " + target.getNickname() + " :" + message + "\r\n";

    target.sendMessage(privmsgFormat);

    std::cout << "[INFO] " << sender.getNickname() << " -> " << target.getNickname() << ": " << message << std::endl;
}

void Privmsg::sendToChannel(Client& sender, Channel& channel, const std::string& message) {
    std::string privmsgFormat = ":" + sender.getNickname() + "!" + sender.getUsername() + "@" + sender.getHostname() + 
                               " PRIVMSG " + channel.getName() + " :" + message + "\r\n";

    // Send to all channel members except the sender
    channel.broadcast(privmsgFormat, &sender);

    std::cout << "[INFO] " << sender.getNickname() << " -> " << channel.getName() << ": " << message << std::endl;
}