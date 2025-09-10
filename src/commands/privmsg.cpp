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

    // Check if message is empty
    if (message.empty()) {
        client.sendMessage("412 " + client.getNickname() + " :No text to send\r\n");
        return;
    }

    // Check if target is a channel (starts with # or &)
    if (isValidChannelName(target)) {
        // Send to channel
        Channel* channel = server.getChannel(target);
        if (!channel) {
            client.sendMessage("403 " + client.getNickname() + " " + target + " :No such channel\r\n");
            return;
        }

        // Check if sender is in the channel
        if (!channel->hasClient(&client)) {
            client.sendMessage("404 " + client.getNickname() + " " + target + " :Cannot send to channel\r\n");
            return;
        }

        sendToChannel(client, *channel, message);
    } else {
        // Send to user
        if (!isValidNickname(target)) {
            client.sendMessage("401 " + client.getNickname() + " " + target + " :No such nick/channel\r\n");
            return;
        }

        // Find target client
        Client* targetClient = server.findClientByNickname(target);
        if (!targetClient) {
            client.sendMessage("401 " + client.getNickname() + " " + target + " :No such nick/channel\r\n");
            return;
        }

        // Can't send message to yourself
        if (targetClient == &client) {
            client.sendMessage("401 " + client.getNickname() + " " + target + " :No such nick/channel\r\n");
            return;
        }

        sendToClient(client, *targetClient, message);
    }
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