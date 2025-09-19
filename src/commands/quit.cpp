#include "../../headers/commands/quit.hpp"
#include "../../headers/network/server.hpp"
#include "../../headers/network/client.hpp"
#include "../../headers/network/channel.hpp"
#include <iostream>

void Quit::execute(Server& server, Client& client, const std::vector<std::string>& params,
                  std::map<int, Client>& clients, std::vector<struct pollfd>& fds, int client_fd) {
    
    // Get quit message (optional parameter)
    std::string quitMessage = params.empty() ? "Client quit" : params[0];
    
    std::cout << "[INFO] Client " << client.getFd() << " (" << client.getNickname() 
              << ") is quitting: " << quitMessage << std::endl;
    
    // Only notify channels if client is authenticated
    if (client.isAuthenticated()) {
        // Notify all channels that this client is quitting
        notifyChannelsOfQuit(server, client, quitMessage);
        
        // Remove client from all channels
        // removeClientFromChannels(server, client);
        
        // // Clean up empty channels
        // cleanupEmptyChannels(server, client);
    }
    
    // Send quit confirmation to client
    client.sendMessage("ERROR :Closing connection: " + quitMessage + "\r\n");
    
    // Remove client from server
    for (size_t i = 1; i < fds.size(); ++i) {
        if (fds[i].fd == client_fd) {
            close(client_fd);
            clients.erase(client_fd);
            fds.erase(fds.begin() + i);
            std::cout << "[INFO] Client " << client_fd << " disconnected and cleaned up" << std::endl;
            break;
        }
    }
}

void Quit::notifyChannelsOfQuit(Server& /* server */, Client& client, const std::string& quitMessage) {
    // Create quit message in IRC format
    std::string quitMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
                         " QUIT :" + quitMessage + "\r\n";
    
    // We need to track which clients we've already notified to avoid duplicates
    // std::set<Client*> notifiedClients;
    
    // This is a simplified approach - in a real implementation, you would iterate through
    // all channels the client is in and notify other members
    // For now, we'll implement a basic version
    

    
    // TODO: Implement proper channel notification when channel membership tracking is available
}
