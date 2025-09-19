#include "../../headers/commands/who.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"

void Who::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // If no parameters, send empty WHO response
    if (params.empty()) {
        sendEndOfWho(client, "*");
        return;
    }

    std::string target = params[0];

    // Check if target is a channel or user
    if (target[0] == '#' || target[0] == '&') {
        // Channel WHO
        handleChannelWho(server, client, target);
    } else {
        // User WHO
        handleUserWho(server, client, target);
    }
}

void Who::handleChannelWho(Server& server, Client& client, const std::string& channelName) {
    Channel* channel = server.getChannel(channelName);
    if (!channel) {
        // Channel doesn't exist
        sendEndOfWho(client, channelName);
        return;
    }

    // Check if client is in the channel (some servers allow WHO without being in channel)
    if (!channel->hasClient(&client)) {
        // For privacy, don't show users if not in channel
        sendEndOfWho(client, channelName);
        return;
    }

    // Send WHO reply for each user in the channel
    const std::set<Client*>& channelClients = channel->getClients();
    for (std::set<Client*>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
        Client* channelClient = *it;
        sendWhoReply(client, channelClient, channelName);
    }

    // Send end of WHO list
    sendEndOfWho(client, channelName);
    

}

void Who::handleUserWho(Server& server, Client& client, const std::string& nickname) {
    Client* targetClient = server.findClientByNickname(nickname);
    if (!targetClient || !targetClient->isAuthenticated()) {
        // User not found or not registered
        sendEndOfWho(client, nickname);
        return;
    }

    // Send WHO reply for the user
    sendWhoReply(client, targetClient, "*");

    // Send end of WHO list
    sendEndOfWho(client, nickname);
    
}

void Who::sendWhoReply(Client& client, Client* targetClient, const std::string& channel) {
    // WHO reply format: 352 <nick> <channel> <user> <host> <server> <nick> <flags> :<hopcount> <real>
    std::string flags = getUserFlags(targetClient, NULL);
    
    std::string whoReply = "352 " + client.getNickname() + " " + channel + " " + 
                          targetClient->getUsername() + " " + targetClient->getHostname() + 
                          " localhost " + targetClient->getNickname() + " " + flags + 
                          " :0 " + targetClient->getRealname();

    client.sendMessage(whoReply + "\r\n");
}

void Who::sendEndOfWho(Client& client, const std::string& target) {
    // End of WHO list: 315 <nick> <target> :End of WHO list
    client.sendMessage("315 " + client.getNickname() + " " + target + " :End of WHO list\r\n");
}

std::string Who::getUserFlags(Client* user, Channel* channel) {
    std::string flags = "H"; // H = Here (not away), G = Gone (away)
    
    // Add channel-specific flags if in a channel
    if (channel) {
        if (channel->isOperator(user)) {
            flags += "@"; // Channel operator
        }
        // You can add more flags here:
        // flags += "+"; // Voice (if you implement voice mode)
    }
    
    return flags;
}