#include "../../headers/commands/topic.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"

void Topic::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // TOPIC command requires at least one parameter (channel name)
    if (params.empty()) {
        client.sendMessage("461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
        return;
    }

    std::string channelName = params[0];

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

    // Check if client is in the channel
    if (!channel->hasClient(&client)) {
        client.sendMessage("442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // If no topic parameter is provided, show current topic
    if (params.size() == 1) {
        showTopic(client, *channel);
        return;
    }

    // Setting a new topic
    std::string newTopic = params[1];

    // Check if topic restriction is enabled and user is not operator
    if (channel->hasTopicRestriction() && !channel->isOperator(&client)) {
        client.sendMessage("482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Set the new topic
    setTopic(client, *channel, newTopic);
}

bool Topic::isValidChannelName(const std::string& channelName) {
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

void Topic::showTopic(Client& client, Channel& channel) {
    if (channel.getTopic().empty()) {
        // No topic is set
        client.sendMessage("331 " + client.getNickname() + " " + channel.getName() + " :No topic is set\r\n");
    } else {
        // Show current topic
        client.sendMessage("332 " + client.getNickname() + " " + channel.getName() + " :" + channel.getTopic() + "\r\n");
        
        // Show who set the topic and when
        client.sendMessage("333 " + client.getNickname() + " " + channel.getName() + " " + 
                          channel.getTopicSetter() + " " + std::to_string(channel.getTopicTime()) + "\r\n");
    }
}

void Topic::setTopic(Client& client, Channel& channel, const std::string& topic) {
    // Set the topic in the channel
    channel.setTopic(topic, client.getNickname());

    // Broadcast the topic change to all channel members
    broadcastTopicChange(client, channel, topic);

    std::cout << "[INFO] " << client.getNickname() << " changed topic of " << channel.getName() << " to: " << topic << std::endl;
}

void Topic::broadcastTopicChange(Client& client, Channel& channel, const std::string& topic) {
    std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
                          " TOPIC " + channel.getName() + " :" + topic + "\r\n";

    // Send to all clients in the channel
    channel.broadcast(topicMsg);
}