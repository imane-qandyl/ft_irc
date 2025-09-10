#include "../../headers/commands/pass.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"

void Pass::execute(Server& server, Client& client, const std::vector<std::string>& params) {
    // Check if client is already authenticated
    if (client.isAuthenticated()) {
        client.sendMessage("462 :You may not reregister\r\n");
        return;
    }

    // PASS command requires exactly 1 parameter: the password
    if (params.empty()) {
        client.sendMessage("461 PASS :Not enough parameters\r\n");
        return;
    }

    std::string providedPassword = params[0];

    // Validate password format (basic validation)
    if (!isValidPassword(providedPassword)) {
        client.sendMessage("464 :Password incorrect\r\n");
        std::cout << "[DEBUG] Client " << client.getFd() << " provided invalid password format" << std::endl;
        return;
    }

    // Check if the provided password matches the server password
    if (providedPassword == server.getPassword()) {
        client.setPasswordProvided(true);
        std::cout << "[DEBUG] Client " << client.getFd() << " provided correct password" << std::endl;
        
        // Check if client can now be registered
        if (!client.getNickname().empty() && !client.getUsername().empty() && !client.isAuthenticated()) {
            client.setAuthenticated(true);
            
            // Send welcome messages
            std::string nick = client.getNickname();
            client.sendMessage("001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
            client.sendMessage("002 " + nick + " :Your host is localhost, running version 1.0\r\n");
            client.sendMessage("003 " + nick + " :This server was created today\r\n");
            client.sendMessage("004 " + nick + " localhost 1.0 o o\r\n");
            
            std::cout << "[INFO] Client " << client.getFd() << " (" << nick << ") successfully registered" << std::endl;
        }
    } else {
        client.sendMessage("464 :Password incorrect\r\n");
        std::cout << "[DEBUG] Client " << client.getFd() << " provided incorrect password" << std::endl;
    }
}

bool Pass::isValidPassword(const std::string& password) {
    // Basic password validation
    if (password.empty() || password.length() > 256) {
        return false;
    }

    // Check for invalid characters (no spaces, control characters, etc.)
    for (size_t i = 0; i < password.length(); ++i) {
        char c = password[i];
        if (c < 33 || c > 126) { // Printable ASCII characters only
            return false;
        }
    }

    return true;
}