#include "../../headers/commands/user.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"

void User::execute(Server& /* server */, Client& client, const std::vector<std::string>& params) {
    // Check if client is already authenticated
    if (client.isAuthenticated()) {
        client.sendMessage("462 :You may not reregister\r\n");
        return;
    }

    // USER command requires exactly 4 parameters: username, hostname, servername, realname
    if (params.size() < 4) {
        client.sendMessage("461 USER :Not enough parameters\r\n");
        return;
    }

    std::string username = params[0];
    std::string hostname = params[1];
    std::string servername = params[2];
    std::string realname = params[3];

    // Validate username format
    if (!isValidUsername(username)) {
        client.sendMessage("461 USER :Invalid username\r\n");
        return;
    }

    // Validate realname format
    if (!isValidRealname(realname)) {
        client.sendMessage("461 USER :Invalid realname\r\n");
        return;
    }

    // Set user information
    client.setUsername(username);
    client.setHostname("localhost"); // Use localhost as hostname for simplicity
    client.setRealname(realname);

    std::cout << "[DEBUG] Client " << client.getFd() << " set user info: " 
              << username << " (" << realname << ")" << std::endl;

    // Check if client can now be registered
    if (client.hasPasswordProvided() && !client.getNickname().empty() && !client.isAuthenticated()) {
        client.setAuthenticated(true);
        
        // Send welcome messages
        std::string nick = client.getNickname();
        client.sendMessage("001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
        client.sendMessage("002 " + nick + " :Your host is localhost, running version 1.0\r\n");
        client.sendMessage("003 " + nick + " :This server was created today\r\n");
        client.sendMessage("004 " + nick + " localhost 1.0 o o\r\n");
        
        std::cout << "[INFO] Client " << client.getFd() << " (" << nick << ") successfully registered" << std::endl;
    }
}

bool User::isValidUsername(const std::string& username) {
    // Username validation
    if (username.empty() || username.length() > 16) {
        return false;
    }

    // Check for valid characters (alphanumeric, underscore, hyphen)
    for (size_t i = 0; i < username.length(); ++i) {
        char c = username[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
    }

    return true;
}

bool User::isValidRealname(const std::string& realname) {
    // Realname validation
    if (realname.empty() || realname.length() > 256) {
        return false;
    }

    // Check for valid characters (printable ASCII)
    for (size_t i = 0; i < realname.length(); ++i) {
        char c = realname[i];
        if (c < 32 || c > 126) { // Allow printable ASCII characters
            return false;
        }
    }

    return true;
}