#include "../../headers/commands/ping.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"

void Ping::execute(Server& /* server */, Client& client, const std::vector<std::string>& params) {
    // PING command requires at least one parameter: the origin
    if (params.empty()) {
        client.sendMessage("409 " + client.getNickname() + " :No origin specified\r\n");
        return;
    }

    std::string origin = params[0];

    // Validate origin format (basic validation)
    if (!isValidOrigin(origin)) {
        client.sendMessage("409 " + client.getNickname() + " :No origin specified\r\n");
        return;
    }

    // Send PONG response
    client.sendMessage("PONG localhost :" + origin + "\r\n");

    std::cout << "[DEBUG] PING from " << client.getNickname() << " (fd " << client.getFd() 
              << ") with origin: " << origin << std::endl;
}

bool Ping::isValidOrigin(const std::string& origin) {
    // Basic origin validation
    if (origin.empty() || origin.length() > 255) {
        return false;
    }

    // Check for valid characters (no control characters)
    for (size_t i = 0; i < origin.length(); ++i) {
        char c = origin[i];
        if (c < 32 || c > 126) { // Only printable ASCII characters
            return false;
        }
    }

    return true;
}