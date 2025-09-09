#include "../../headers/commands/mode.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"
#include <sstream>

void Mode::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is authenticated
    if (!client.isAuthenticated()) {
        client.sendMessage("451 * :You have not registered\r\n");
        return;
    }

    // MODE command requires at least one parameter (channel name)
    if (params.empty()) {
        client.sendMessage("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string target = params[0];

    // Check if it's a channel mode (starts with # or &)
    if (!isValidChannelName(target)) {
        client.sendMessage("403 " + client.getNickname() + " " + target + " :No such channel\r\n");
        return;
    }

    // Get channel
    Channel* channel = server.getChannel(target);
    if (!channel) {
        client.sendMessage("403 " + client.getNickname() + " " + target + " :No such channel\r\n");
        return;
    }

    // Check if client is in the channel
    if (!channel->hasClient(&client)) {
        client.sendMessage("442 " + client.getNickname() + " " + target + " :You're not on that channel\r\n");
        return;
    }

    // If only channel name is provided, show current modes
    if (params.size() == 1) {
        std::string modes = "+";
        std::string args = "";
        
        if (channel->isInviteOnly()) {
            modes += "i";
        }
        if (channel->hasTopicRestriction()) {
            modes += "t";
        }
        if (channel->hasKey()) {
            modes += "k";
            if (channel->isOperator(&client)) {
                args += " " + channel->getKey();
            }
        }
        if (channel->hasUserLimit()) {
            modes += "l";
            std::stringstream ss;
            ss << channel->getUserLimit();
            args += " " + ss.str();
        }
        
        if (modes == "+") {
            modes = "+";
        }
        
        client.sendMessage("324 " + client.getNickname() + " " + target + " " + modes + args + "\r\n");
        return;
    }

    // Check if client is operator (required for mode changes)
    if (!channel->isOperator(&client)) {
        client.sendMessage("482 " + client.getNickname() + " " + target + " :You're not channel operator\r\n");
        return;
    }

    // Process mode changes
    std::string modeString = params[1];
    std::vector<std::string> args;
    for (size_t i = 2; i < params.size(); ++i) {
        args.push_back(params[i]);
    }

    processChannelModes(server, client, *channel, modeString, args);
}

bool Mode::isValidChannelName(const std::string& channelName) {
    if (channelName.empty() || channelName.length() > 50) {
        return false;
    }
    
    // Channel names must start with # or &
    if (channelName[0] != '#' && channelName[0] != '&') {
        return false;
    }
    
    return true;
}

// KEEP ONLY ONE SET OF THESE METHODS (with /* client */ to suppress warnings):
void Mode::handleInviteOnly(Client& /* client */, Channel& channel, bool add, std::string& response) {
    if (add) {
        if (!channel.isInviteOnly()) {
            channel.setInviteOnly(true);
            response += "+i";
        }
    } else {
        if (channel.isInviteOnly()) {
            channel.setInviteOnly(false);
            response += "-i";
        }
    }
}

void Mode::handleTopicRestriction(Client& /* client */, Channel& channel, bool add, std::string& response) {
    if (add) {
        if (!channel.hasTopicRestriction()) {
            channel.setTopicRestriction(true);
            response += "+t";
        }
    } else {
        if (channel.hasTopicRestriction()) {
            channel.setTopicRestriction(false);
            response += "-t";
        }
    }
}

void Mode::handleChannelKey(Client& /* client */, Channel& channel, bool add, const std::string& key, std::string& response) {
    if (add) {
        if (key.empty()) {
            return;
        }
        channel.setKey(key);
        response += "+k " + key;
    } else {
        if (channel.hasKey()) {
            channel.removeKey();
            response += "-k";
        }
    }
}

void Mode::processChannelModes(Server& server, Client& client, Channel& channel, 
                              const std::string& modeString, const std::vector<std::string>& args) {
    bool adding = true;
    size_t argIndex = 0;
    std::string response = "";
    
    for (size_t i = 0; i < modeString.length(); ++i) {
        char mode = modeString[i];
        
        if (mode == '+') {
            adding = true;
            continue;
        } else if (mode == '-') {
            adding = false;
            continue;
        }
        
        std::string arg = "";
        if (argIndex < args.size()) {
            arg = args[argIndex];
        }
        
        switch (mode) {
            case 'i':
                handleInviteOnly(client, channel, adding, response);
                break;
            case 't':
                handleTopicRestriction(client, channel, adding, response);
                break;
            case 'k':
                if (adding && arg.empty()) {
                    client.sendMessage("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                    continue;
                }
                handleChannelKey(client, channel, adding, arg, response);
                if (adding || (!adding && channel.hasKey())) {
                    argIndex++;
                }
                break;
            case 'o':
                if (arg.empty()) {
                    client.sendMessage("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                    continue;
                }
                handleOperatorPrivilege(server, client, channel, adding, arg, response);
                argIndex++;
                break;
            case 'l':
                if (adding && arg.empty()) {
                    client.sendMessage("461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                    continue;
                }
                handleUserLimit(client, channel, adding, arg, response);
                if (adding) {
                    argIndex++;
                }
                break;
            default:
                client.sendMessage("472 " + client.getNickname() + " " + mode + " :is unknown mode char to me\r\n");
                break;
        }
    }
    
    if (!response.empty()) {
        sendModeResponse(client, channel, response);
    }
}

void Mode::handleOperatorPrivilege(Server& server, Client& client, Channel& channel, bool add, const std::string& nickname, std::string& response) {
    if (!isValidNickname(nickname)) {
        client.sendMessage("401 " + client.getNickname() + " " + nickname + " :No such nick/channel\r\n");
        return;
    }
    
    Client* targetClient = server.findClientByNickname(nickname);
    if (!targetClient) {
        client.sendMessage("401 " + client.getNickname() + " " + nickname + " :No such nick/channel\r\n");
        return;
    }
    
    if (!channel.hasClient(targetClient)) {
        client.sendMessage("441 " + client.getNickname() + " " + nickname + " " + channel.getName() + " :They aren't on that channel\r\n");
        return;
    }
    
    if (add) {
        if (!channel.isOperator(targetClient)) {
            channel.addOperator(targetClient);
            response += "+o " + nickname;
        }
    } else {
        if (channel.isOperator(targetClient)) {
            channel.removeOperator(targetClient);
            response += "-o " + nickname;
        }
    }
}

void Mode::handleUserLimit(Client& /* client */, Channel& channel, bool add, const std::string& limit, std::string& response) {
    if (add) {
        if (limit.empty() || !isNumeric(limit)) {
            return;
        }
        
        int limitValue = std::atoi(limit.c_str());
        if (limitValue <= 0 || limitValue > 1000) {
            return;
        }
        
        channel.setUserLimit(limitValue);
        response += "+l " + limit;
    } else {
        if (channel.hasUserLimit()) {
            channel.removeUserLimit();
            response += "-l";
        }
    }
}

void Mode::sendModeResponse(Client& client, Channel& channel, const std::string& modeChanges) {
    if (modeChanges.empty()) {
        return;
    }
    
    std::string modeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
                         " MODE " + channel.getName() + " " + modeChanges + "\r\n";
    
    // Broadcast to all channel members
    channel.broadcast(modeMsg);
}

bool Mode::isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }
    
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

bool Mode::isNumeric(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    
    return true;
}

// REMOVE ALL THE DUPLICATE DEFINITIONS THAT WERE AT THE END OF THE FILE