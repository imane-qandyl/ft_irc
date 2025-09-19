#include "../../headers/commands/nick.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"

void Nick::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // NICK command requires exactly 1 parameter: the new nickname
    if (params.empty()) {
        client.sendMessage("431 :No nickname given\r\n");
        return;
    }

    std::string newNick = params[0];

    // Validate nickname format
    if (!isValidNickname(newNick)) {
        client.sendMessage("432 " + newNick + " :Erroneous nickname\r\n");
        return;
    }

    // Check if nickname is already in use by another client
    if (isNicknameInUse(server, newNick) && client.getNickname() != newNick) {
        client.sendMessage("433 " + newNick + " :Nickname is already in use\r\n");
        return;
    }

    std::string oldNick = client.getNickname();

    // Set the new nickname
    client.setNickname(newNick);

    // If this is the initial nickname setting (registration)
    if (oldNick.empty()) {
        
        // Check if client can now be registered
        if (client.hasPasswordProvided() && !client.getUsername().empty() && !client.isAuthenticated()) {
            client.setAuthenticated(true);
            
            // Send welcome messages
            client.sendMessage("001 " + newNick + " :Welcome to the IRC Network " + newNick + "\r\n");
            client.sendMessage("002 " + newNick + " :Your host is localhost, running version 1.0\r\n");
            client.sendMessage("003 " + newNick + " :This server was created today\r\n");
            client.sendMessage("004 " + newNick + " localhost 1.0 o o\r\n");
            
            std::cout << "[INFO] Client " << client.getFd() << " (" << newNick << ") successfully registered" << std::endl;
        }
    } else {
        // This is a nickname change
        
        // Send nick change notifications
        sendNickChangeNotifications(server, client, oldNick, newNick);
    }
}

bool Nick::isValidNickname(const std::string& nickname) {
    // Check length (1-9 characters as per IRC RFC)
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }

    // First character must be a letter or special character
    char first = nickname[0];
    if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') || 
          first == '[' || first == ']' || first == '\\' || first == '`' || 
          first == '^' || first == '{' || first == '}' || first == '_')) {
        return false;
    }

    // Check remaining characters
    for (size_t i = 1; i < nickname.length(); ++i) {
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

bool Nick::isNicknameInUse(Server& server, const std::string& nickname) {
    return server.findClientByNickname(nickname) != NULL;
}

void Nick::sendNickChangeNotifications(Server& /* server */, Client& client, const std::string& oldNick, const std::string& newNick) {
    std::string nickChangeMsg = ":" + oldNick + "!" + client.getUsername() + "@" + client.getHostname() + 
                               " NICK :" + newNick + "\r\n";

    // Send to the client themselves
    client.sendMessage(nickChangeMsg);

    // TODO: Send to all users who share channels with this client
    // This would require implementing proper channel membership tracking
    
    std::cout << "[INFO] Nick change notification sent: " << oldNick << " -> " << newNick << std::endl;
}