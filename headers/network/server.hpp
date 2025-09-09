#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <sstream>
#include <algorithm>
#include "client.hpp"
#include "channel.hpp"
#include "../commands/join.hpp"
#include "../commands/invite.hpp"
#include "../commands/leave.hpp"
#include "../commands/mode.hpp"
#include "../commands/topic.hpp"  // Add this include

class Server {
private:
    int _port;
    std::string _password;
    int _server_fd;
    std::map<std::string, Channel*> _channels;
    std::map<int, Client>* _currentClients;  // Add this line

public:
    Server(int port, const std::string& password);
    ~Server();
    
    void setupSocket();
    void run();
    
    // Channel management
    Channel* getChannel(const std::string& name);
    Channel* createChannel(const std::string& name);
    void removeChannel(const std::string& name);


    Client* findClientByNickname(const std::string& nickname);
    void setCurrentClients(std::map<int, Client>* clients); // Add this line
    
    
private:
    void initializeServer(std::vector<struct pollfd>& fds);
    bool handlePollError(int ret);
    void handleClientTimeouts(std::map<int, Client>& clients, std::map<int, time_t>& client_last_activity, 
                            std::vector<struct pollfd>& fds, time_t current_time);
    void handleNewConnection(std::vector<struct pollfd>& fds, std::map<int, Client>& clients,
                           std::map<int, time_t>& client_last_activity, time_t current_time);
    void handleClientActivity(std::vector<struct pollfd>& fds, std::map<int, Client>& clients,
                            std::map<int, time_t>& client_last_activity, time_t current_time);
    void disconnectClient(int client_fd, std::map<int, Client>& clients, std::map<int, time_t>& client_last_activity,
                        std::vector<struct pollfd>& fds, size_t index);
    void handleClientSend(struct pollfd& pfd, Client& client, std::map<int, time_t>& client_last_activity, time_t current_time);
    bool handleClientReceive(int client_fd, std::map<int, Client>& clients, std::map<int, time_t>& client_last_activity,
                           std::vector<struct pollfd>& fds, time_t current_time, size_t index);
    void getClientIP(const struct sockaddr_storage& their_addr, char* client_ip);
    void shutdownServer(std::vector<struct pollfd>& fds);
    void processIRCCommand(int client_fd, const std::string& message, std::map<int, Client>& clients, std::vector<struct pollfd>& fds);
      // Add these new method declarations
      void handlePassCommand(Client& client, const std::vector<std::string>& params);
      void handleNickCommand(Client& client, const std::vector<std::string>& params);
      void handleUserCommand(Client& client, const std::vector<std::string>& params);
      void handlePingCommand(Client& client, const std::vector<std::string>& params);
      void handleQuitCommand(Client& client, const std::vector<std::string>& params, 
                            std::map<int, Client>& clients, std::vector<struct pollfd>& fds, int client_fd);
      void checkClientRegistration(Client& client);
};

#endif