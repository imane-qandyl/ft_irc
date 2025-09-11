#include "server.hpp"
#include "signals.hpp"

// Define MSG_NOSIGNAL if not available (macOS compatibility)
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _server_fd(-1), _currentClients(NULL) { // Add _currentClients(NULL)
    setupSignalHandlers();
}

// Add this method:
void Server::setCurrentClients(std::map<int, Client>* clients) {
    _currentClients = clients;
}

Server::~Server() {
    if (_server_fd != -1)
        close(_server_fd);
}

// Channel management methods
Channel* Server::getChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end()) {
        return it->second;
    }
    return NULL;
}

Channel* Server::createChannel(const std::string& name) {
    // Check if channel already exists
    Channel* existing = getChannel(name);
    if (existing) {
        return existing;
    }
    
    // Create new channel
    Channel* newChannel = new Channel(name);
    if (newChannel) {
        _channels[name] = newChannel;
        std::cout << "[INFO] Created new channel: " << name << std::endl;
        return newChannel;
    }
    
    std::cerr << "[ERROR] Failed to create channel: " << name << std::endl;
    return NULL;
}

void Server::removeChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end()) {
        delete it->second;
        _channels.erase(it);
        std::cout << "[INFO] Removed channel: " << name << std::endl;
    }
}

void Server::setupSocket() {
    struct addrinfo hints, *servinfo, *p;// will point to the results
    int yes = 1;
    int rv;
    // removed char portStr[16];
    std::memset(&hints, 0, sizeof hints);// make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Use my IP

    std::string portStr = std::to_string(_port);
    if ((rv = getaddrinfo(NULL, portStr.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        _server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_server_fd < 0)
            continue;

        if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            close(_server_fd);
            continue;
        }

        if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0) {
            close(_server_fd);
            continue;
        }

        if (bind(_server_fd, p->ai_addr, p->ai_addrlen) < 0) {
            close(_server_fd);
            continue;
        }

        break; // Success
    }

    if (p == NULL) {
        std::cerr << "server: failed to bind" << std::endl;
        freeaddrinfo(servinfo);
        exit(1);
    }

    if (listen(_server_fd, SOMAXCONN) < 0) {
        std::cerr << "Error: Cannot listen on socket." << std::endl;
        close(_server_fd);
        freeaddrinfo(servinfo);
        exit(1);
    }

    freeaddrinfo(servinfo);
}

void Server::run() {
    std::vector<struct pollfd> fds;
    std::map<int, Client> clients;
    std::map<int, time_t> client_last_activity;
    
    initializeServer(fds);
    
    while (!g_shutdown) {
        int ret = poll(&fds[0], fds.size(), 1000);
        if (handlePollError(ret)) break;
        
        time_t current_time = time(NULL);
        handleClientTimeouts(clients, client_last_activity, fds, current_time);
        
        if (ret == 0) continue;
        
        if (fds[0].revents & POLLIN) {
            handleNewConnection(fds, clients, client_last_activity, current_time);
        }
        
        handleClientActivity(fds, clients, client_last_activity, current_time);
    }
    
    shutdownServer(fds);
}

void Server::initializeServer(std::vector<struct pollfd>& fds) {
    struct pollfd server_pollfd;
    server_pollfd.fd = _server_fd;
    server_pollfd.events = POLLIN;
    fds.push_back(server_pollfd);
    
    std::cout << "[INFO] Server started on port " << _port << std::endl;
    std::cout << "[INFO] Use CTRL+C for graceful shutdown" << std::endl;
}

bool Server::handlePollError(int ret) {
    if (ret < 0) {
        if (errno == EINTR) {
            std::cout << "[INFO] Poll interrupted by signal" << std::endl;
            return false;
        }
        std::cerr << "[ERROR] Poll failed: " << strerror(errno) << std::endl;
        return true;
    }
    return false;
}

void Server::handleClientTimeouts(std::map<int, Client>& clients, std::map<int, time_t>& client_last_activity, 
                                std::vector<struct pollfd>& fds, time_t current_time) {
    const int CLIENT_TIMEOUT = 300;
    std::map<int, time_t>::iterator it = client_last_activity.begin();
    
    while (it != client_last_activity.end()) {
        if (current_time - it->second > CLIENT_TIMEOUT) {
            int timeout_fd = it->first;
            std::cout << "[INFO] Client fd " << timeout_fd << " timed out" << std::endl;
            
            for (size_t i = 1; i < fds.size(); ++i) {
                if (fds[i].fd == timeout_fd) {
                    close(timeout_fd);
                    clients.erase(timeout_fd);
                    fds.erase(fds.begin() + i);
                    break;
                }
            }
            client_last_activity.erase(it++);
        } else {
            ++it;
        }
    }
}

void Server::handleNewConnection(std::vector<struct pollfd>& fds, std::map<int, Client>& clients,
                               std::map<int, time_t>& client_last_activity, time_t current_time) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(_server_fd, (struct sockaddr *)&their_addr, &addr_size);
    
    if (new_fd >= 0) {
        char client_ip[INET6_ADDRSTRLEN];
        getClientIP(their_addr, client_ip);
        
        if (fcntl(new_fd, F_SETFL, O_NONBLOCK) < 0) {
            std::cerr << "[ERROR] Failed to set non-blocking mode for client " << new_fd << std::endl;
            close(new_fd);
            return;
        }
        
        std::cout << "[INFO] New client connected: fd " << new_fd << " from " << client_ip << std::endl;
        
        struct pollfd client_pollfd;
        client_pollfd.fd = new_fd;
        client_pollfd.events = POLLIN | POLLOUT;
        fds.push_back(client_pollfd);
        
        clients.insert(std::make_pair(new_fd, Client(new_fd)));
        client_last_activity[new_fd] = current_time;
        
    // Do not send welcome here; send only after registration (see checkClientRegistration)
    }
}

void Server::handleClientActivity(std::vector<struct pollfd>& fds, std::map<int, Client>& clients,
                                std::map<int, time_t>& client_last_activity, time_t current_time) {
    for (size_t i = 1; i < fds.size(); ++i) {
        int client_fd = fds[i].fd;
        
        if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            disconnectClient(client_fd, clients, client_last_activity, fds, i);
            --i;
            continue;
        }
        
        if (fds[i].revents & POLLOUT) {
            handleClientSend(fds[i], clients[client_fd], client_last_activity, current_time);
        }
        
        if (fds[i].revents & POLLIN) {
            if (handleClientReceive(client_fd, clients, client_last_activity, fds, current_time, i)) {
                --i;
            }
        }
    }
}

void Server::disconnectClient(int client_fd, std::map<int, Client>& clients, std::map<int, time_t>& client_last_activity,
                            std::vector<struct pollfd>& fds, size_t index) {
    std::cout << "[INFO] Client fd " << client_fd << " disconnected" << std::endl;
    close(client_fd);
    clients.erase(client_fd);
    client_last_activity.erase(client_fd);
    fds.erase(fds.begin() + index);
}

void Server::handleClientSend(struct pollfd& pfd, Client& client, std::map<int, time_t>& client_last_activity, time_t current_time) {
    if (client.hasDataToSend()) {
        std::string& sendBuf = client.getSendBuffer();
        ssize_t sent = send(pfd.fd, sendBuf.c_str(), sendBuf.size(), MSG_NOSIGNAL);
        
        if (sent > 0) {
            client.removeSentFromBuffer(sent);
            client_last_activity[pfd.fd] = current_time;
            
            if (!client.hasDataToSend()) {
                pfd.events = POLLIN;
            }
        }
    } else {
        pfd.events = POLLIN;
    }
}

bool Server::handleClientReceive(int client_fd, std::map<int, Client>& clients, std::map<int, time_t>& client_last_activity,
                               std::vector<struct pollfd>& fds, time_t current_time, size_t index) {
    char buf[512];
    ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    
    if (n > 0) {
        buf[n] = '\0';
        client_last_activity[client_fd] = current_time;
        clients[client_fd].appendToBuffer(std::string(buf, n));
        clients[client_fd].markReceivedData();
        
        while (clients[client_fd].hasCompleteMessage()) {
            std::string message = clients[client_fd].extractMessage();
            processIRCCommand(client_fd, message, clients, fds);
        }
        return false;
    } else if (n == 0 && clients[client_fd].hasReceivedData()) {
        disconnectClient(client_fd, clients, client_last_activity, fds, index);
        return true;
    }
    return false;
}

void Server::getClientIP(const struct sockaddr_storage& their_addr, char* client_ip) {
    void* addr_ptr;
    if (their_addr.ss_family == AF_INET) {
        addr_ptr = &((struct sockaddr_in*)&their_addr)->sin_addr;
    } else {
        addr_ptr = &((struct sockaddr_in6*)&their_addr)->sin6_addr;
    }
    
    if (inet_ntop(their_addr.ss_family, addr_ptr, client_ip, INET6_ADDRSTRLEN) == NULL) {
        strcpy(client_ip, "unknown");
    }
}

void Server::shutdownServer(std::vector<struct pollfd>& fds) {
    std::cout << "[INFO] Shutting down server gracefully..." << std::endl;
    for (size_t i = 1; i < fds.size(); ++i) {
        std::string goodbye = "Server shutting down. Goodbye!\r\n";
        send(fds[i].fd, goodbye.c_str(), goodbye.length(), MSG_NOSIGNAL);
        close(fds[i].fd);
    }
    std::cout << "[INFO] Server shutdown complete" << std::endl;
}

void Server::processIRCCommand(int client_fd, const std::string& message, std::map<int, Client>& clients, std::vector<struct pollfd>& fds) {
    setCurrentClients(&clients);
    
    // Parse the IRC command
    std::istringstream iss(message);
    std::string command;
    std::vector<std::string> params;
    
    // Extract command
    if (!(iss >> command)) {
        return; // Empty message
    }
    
    // Convert command to uppercase for case-insensitive comparison
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
    
    // Extract parameters
    std::string param;
    while (iss >> param) {
        // Handle parameters that start with ':'
        if (param[0] == ':') {
            // Rest of the line is one parameter
            std::string rest;
            std::getline(iss, rest);
            param = param.substr(1) + rest; // Remove ':' and add rest
            params.push_back(param);
            break;
        }
        params.push_back(param);
    }
    
    // Get client reference
    Client& client = clients[client_fd];
    
    // Handle different IRC commands
    if (command == "JOIN") {
        Join::execute(*this, client, params);
    }
    else if (command == "INVITE") {  // Add this
        Invite::execute(*this, client, params);
    }
    else if (command == "PART") {  // Add this - PART is the standard IRC command for leaving
        Leave::execute(*this, client, params);
    }
    else if (command == "LEAVE") {  // Add this as alias for PART
        Leave::execute(*this, client, params);
    }
    else if (command == "MODE") {
        Mode::execute(*this, client, params);
    }
    else if (command == "TOPIC") {  // Add this line
        Topic::execute(*this, client, params);
    }
    else if (command == "PASS") {
        handlePassCommand(client, params);
    }
    else if (command == "NICK") {
        handleNickCommand(client, params);
    }
    else if (command == "USER") {
        handleUserCommand(client, params);
    }
    else if (command == "PING") {
        handlePingCommand(client, params);
    }
    else if (command == "QUIT") {
        handleQuitCommand(client, params, clients, fds, client_fd);
    }
    else {
        // Unknown command
        client.sendMessage("421 " + client.getNickname() + " " + command + " :Unknown command\r\n");
    }
    setCurrentClients(NULL);
    // Switch to POLLOUT if we have data to send
    for (size_t i = 1; i < fds.size(); ++i) {
        if (fds[i].fd == client_fd && client.hasDataToSend()) {
            fds[i].events = POLLIN | POLLOUT;
            break;
        }
    }
}

// Add these helper methods to handle basic IRC commands
void Server::handlePassCommand(Client& client, const std::vector<std::string>& params) {
    if (params.empty()) {
        client.sendMessage("461 PASS :Not enough parameters\r\n");
        return;
    }
    
    if (params[0] == _password) {
        client.setPasswordProvided(true);
    } else {
        client.sendMessage("464 :Password incorrect\r\n");
    }
}

void Server::handleNickCommand(Client& client, const std::vector<std::string>& params) {
    if (params.empty()) {
        client.sendMessage("431 :No nickname given\r\n");
        return;
    }
    
    std::string nick = params[0];
    
    // Basic nickname validation
    if (nick.empty() || nick.length() > 9) {
        client.sendMessage("432 " + nick + " :Erroneous nickname\r\n");
        return;
    }
    
    checkClientRegistration(client);
}

void Server::handleUserCommand(Client& client, const std::vector<std::string>& params) {
    if (params.size() < 4) {
        client.sendMessage("461 USER :Not enough parameters\r\n");
        return;
    }
    
    client.setUsername(params[0]);
    client.setHostname("localhost"); // Simplified
    client.setRealname(params[3]);
        
    checkClientRegistration(client);
}

void Server::handlePingCommand(Client& client, const std::vector<std::string>& params) {
    if (params.empty()) {
        client.sendMessage("409 :No origin specified\r\n");
        return;
    }
    
    client.sendMessage("PONG :" + params[0] + "\r\n");
}

void Server::handleQuitCommand(Client& client, const std::vector<std::string>& params, 
                              std::map<int, Client>& clients, std::vector<struct pollfd>& fds, int client_fd) {
    std::string quitMsg = params.empty() ? "Client quit" : params[0];
    
    // Remove client from all channels
    // TODO: Implement channel cleanup
    
    client.sendMessage("ERROR :Closing connection: " + quitMsg + "\r\n");
    
    // Disconnect the client
    for (size_t i = 1; i < fds.size(); ++i) {
        if (fds[i].fd == client_fd) {
            close(client_fd);
            clients.erase(client_fd);
            fds.erase(fds.begin() + i);
            break;
        }
    }
}

void Server::checkClientRegistration(Client& client) {
    // Only allow registration if password is provided and both nick and user are set
    std::cout << "[DEBUG] checkClientRegistration: hasPasswordProvided=" << client.hasPasswordProvided()
              << ", nick='" << client.getNickname() << "', user='" << client.getUsername() << "', state=" << client.getState() << std::endl;
    if (client.hasPasswordProvided() && !client.getNickname().empty() && !client.getUsername().empty()) {
        if (client.getState() != REGISTERED) {
            std::cout << "[DEBUG] Registering client " << client.getFd() << std::endl;
            client.setState(REGISTERED);
            // Send welcome messages
            std::string nick = client.getNickname();
            client.sendMessage("001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
            client.sendMessage("002 " + nick + " :Your host is localhost, running version 1.0\r\n");
            client.sendMessage("003 " + nick + " :This server was created today\r\n");
            client.sendMessage("004 " + nick + " localhost 1.0 o o\r\n");
            std::cout << "[INFO] Client " << client.getFd() << " (" << nick << ") successfully registered" << std::endl;
        }
    }
}

// Replace the broken findClientByNickname method:
Client* Server::findClientByNickname(const std::string& nickname) {
    if (!_currentClients) return NULL;
    
    for (std::map<int, Client>::iterator it = _currentClients->begin(); it != _currentClients->end(); ++it) {
        if (it->second.getNickname() == nickname) {
            return &(it->second);
        }
    }
    return NULL;
}