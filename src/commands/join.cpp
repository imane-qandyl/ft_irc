#include "../../headers/commands/join.hpp"
#include <sstream>

void Join::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // JOIN command requires at least one parameter (channel name)
    if (params.empty() || params[0].empty()) {
        client.sendMessage("461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }

    std::string channels = params[0];
    std::string keys = params.size() > 1 ? params[1] : "";

    // Parse comma-separated channels and keys
    std::vector<std::string> channelList;
    std::vector<std::string> keyList;
    
    std::stringstream channelStream(channels);
    std::string channel;
    while (std::getline(channelStream, channel, ',')) {
        if (!channel.empty()) {
            channelList.push_back(channel);
        }
    }

    std::stringstream keyStream(keys);
    std::string key;
    while (std::getline(keyStream, key, ',')) {
        keyList.push_back(key);
    }

    // Join each channel
    for (size_t i = 0; i < channelList.size(); ++i) {
        std::string channelName = channelList[i];
        std::string channelKey = (i < keyList.size()) ? keyList[i] : "";
        
        if (!isValidChannelName(channelName)) {
            client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
            continue;
        }

        joinChannel(server, client, channelName, channelKey);
    }
}

bool Join::isValidChannelName(const std::string& name) {
    if (name.empty() || name.length() > 50) {
        return false;
    }
    
    // Channel names must start with # or &
    if (name[0] != '#' && name[0] != '&') {
        return false;
    }
    
    // Check for invalid characters (space, comma, bell) - use traditional for loop
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (c == ' ' || c == ',' || c == '\007') {
            return false;
        }
    }
    
    return true;
}

void Join::joinChannel(Server& server, Client& client, const std::string& channelName, const std::string& key) {
    Channel* channel = server.getChannel(channelName);
    
    // Create channel if it doesn't exist
    if (!channel) {
        channel = server.createChannel(channelName);
        if (!channel) {
            client.sendMessage("403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
            return;
        }
        // First user becomes operator
        channel->addOperator(&client);
    }
    
    // Check if client is already in the channel
    if (channel->hasClient(&client)) {
        return; // Already in channel, do nothing
    }
    
    // Check channel key if required
    if (channel->hasKey() && channel->getKey() != key) {
        client.sendMessage("475 " + client.getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n");
        return;
    }
    
    // Check if channel is invite-only
    if (channel->isInviteOnly() && !channel->isInvited(&client)) {
        client.sendMessage("473 " + client.getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n");
        return;
    }
    
    // Check user limit
    if (channel->hasUserLimit() && channel->getClientCount() >= channel->getUserLimit()) {
        client.sendMessage("471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n");
        return;
    }
    
    // Add client to channel
    channel->addClient(&client);
    client.addChannel(channel);
    
    // Send JOIN messages
    sendJoinMessages(server, client, *channel);
    
    // Send channel information
    sendChannelInfo(server, client, *channel);
}

void Join::sendJoinMessages(Server& /* server */, Client& client, Channel& channel) {
    std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
                         " JOIN " + channel.getName() + "\r\n";
    
    // Send JOIN message to all clients in the channel (including the joining client)
    channel.broadcast(joinMsg);
}

void Join::sendChannelInfo(Server& /* server */, Client& client, Channel& channel) {
    // Send topic if it exists
    if (!channel.getTopic().empty()) {
        client.sendMessage("332 " + client.getNickname() + " " + channel.getName() + " :" + channel.getTopic() + "\r\n");
        client.sendMessage("333 " + client.getNickname() + " " + channel.getName() + " " + channel.getTopicSetter() + 
                          " " + std::to_string(channel.getTopicTime()) + "\r\n");
    } else {
        client.sendMessage("331 " + client.getNickname() + " " + channel.getName() + " :No topic is set\r\n");
    }
    
    // Send user list (NAMES reply)
    std::string namesList = "353 " + client.getNickname() + " = " + channel.getName() + " :";
    const std::set<Client*>& clients = channel.getClients();
    
    bool first = true;
    for (std::set<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (!first) {
            namesList += " ";
        }
        if (channel.isOperator(*it)) {
            namesList += "@";
        }
        namesList += (*it)->getNickname();
        first = false;
    }
    namesList += "\r\n";
    
    client.sendMessage(namesList);
    
    // End of NAMES list
    client.sendMessage("366 " + client.getNickname() + " " + channel.getName() + " :End of /NAMES list\r\n");
}