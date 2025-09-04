#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <time.h>
#include <arpa/inet.h>
#include "client.hpp"

class Server {
private:
    int _port;
    std::string _password;
    int _server_fd;
    
    void processIRCCommand(int client_fd, const std::string& message, std::map<int, Client>& clients, std::vector<struct pollfd>& fds);
    
    // Helper methods for run()
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

public:
    Server(int port, const std::string& password);
    ~Server();
    
    void setupSocket();
    void run();
};

#endif