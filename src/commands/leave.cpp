#include "../../headers/commands/leave.hpp"

void Leave::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // PART command requires at least one parameter (channel name)
    if (params.empty()) {
        client.sendMessage("461 " + client.getNickname() + " PART :Not enough parameters\r\n");
        return;
    }

    std::string channels = params[0];
    std::string reason = params.size() > 1 ? params[1] : "Leaving";

    // Parse comma-separated channels
    std::vector<std::string> channelList;
    std::stringstream channelStream(channels);
    std::string channel;
    
    while (std::getline(channelStream, channel, ',')) {
        if (!channel.empty()) {
            channelList.push_back(channel);
        }
    }

    // Leave each channel
    for (size_t i = 0; i < channelList.size(); ++i) {
        std::string channelName = channelList[i];
        
        if (!isValidChannelName(channelName)) {
            client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
            continue;
        }

        leaveChannel(server, client, channelName, reason);
    }
}

bool Leave::isValidChannelName(const std::string& channelName) {
    if (channelName.empty() || channelName.length() > 50) {
        return false;
    }
    
    // Channel names must start with # or &
    if (channelName[0] != '#' && channelName[0] != '&') {
        return false;
    }
    
    // Check for invalid characters (space, comma, bell)
    for (size_t i = 0; i < channelName.length(); ++i) {
        char c = channelName[i];
        if (c == ' ' || c == ',' || c == '\007') {
            return false;
        }
    }
    
    return true;
}

void Leave::leaveChannel(Server& server, Client& client, const std::string& channelName, const std::string& reason) {
    Channel* channel = server.getChannel(channelName);
    
    // Check if channel exists
    if (!channel) {
        client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }
    
    // Check if client is in the channel
    if (!channel->hasClient(&client)) {
        client.sendMessage("442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }
    
    // Send PART messages before removing the client
    sendPartMessages(server, client, *channel, reason);
    
    // Remove client from channel
    channel->removeClient(&client);
    client.removeChannel(channel);
    
    std::cout << "[INFO] Client " << client.getNickname() << " left channel " << channelName << std::endl;
    
    // Remove channel if it's empty
    if (channel->getClientCount() == 0) {
        server.removeChannel(channelName);
        std::cout << "[INFO] Channel " << channelName << " removed (empty)" << std::endl;
    }
}

void Leave::sendPartMessages(Server& /* server */, Client& client, Channel& channel, const std::string& reason) {
    std::string partMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
                         " PART " + channel.getName();
    
    if (!reason.empty()) {
        partMsg += " :" + reason;
    }
    partMsg += "\r\n";
    
    // Send PART message to all clients in the channel (including the leaving client)
    channel.broadcast(partMsg);
}