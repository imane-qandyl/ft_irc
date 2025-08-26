#include "../headers/server.hpp"

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _server_fd(-1) {}

Server::~Server() {
    if (_server_fd != -1)
        close(_server_fd);
}

void Server::setupSocket() {
    struct addrinfo hints, *servinfo, *p;// will point to the results
    int yes = 1;
    int rv;
    char portStr[16];
    std::memset(&hints, 0, sizeof hints);// make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Use my IP

    snprintf(portStr, sizeof(portStr), "%d", _port);
    if ((rv = getaddrinfo(NULL, portStr, &hints, &servinfo)) != 0) {
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
    std::map<int, Client> clients; // Map fd to Client object
    // Add the server socket to pollfd vector
    struct pollfd server_pollfd;
    server_pollfd.fd = _server_fd;
    server_pollfd.events = POLLIN;
    fds.push_back(server_pollfd);

    while (true) {
        int ret = poll(&fds[0], fds.size(), -1);
        if (ret < 0) {
            std::cerr << "poll error" << std::endl;
            break;
        }
        // Check for new connections
        if (fds[0].revents & POLLIN) {
            struct sockaddr_storage their_addr;
            socklen_t addr_size = sizeof(their_addr);
            int new_fd = accept(_server_fd, (struct sockaddr *)&their_addr, &addr_size);
            if (new_fd >= 0) {
                // Set the new socket to non-blocking mode
                if (fcntl(new_fd, F_SETFL, O_NONBLOCK) < 0) {
                    std::cerr << "fcntl error on new client socket" << std::endl;
                    close(new_fd);
                } else {
                    std::cout << "New client connected: fd " << new_fd << std::endl;
                    struct pollfd client_pollfd;
                    client_pollfd.fd = new_fd;
                    client_pollfd.events = POLLIN | POLLOUT; // Will send on next POLLOUT event
                    fds.push_back(client_pollfd);
                    clients[new_fd] = Client(new_fd);
                    clients[new_fd].appendToSendBuffer("Welcome to the IRC server!\r\n");
                    std::cout << "[DEBUG] Welcome message queued for fd " << new_fd << std::endl;
                }
            }
        }
        // Check for activity on client sockets
        for (size_t i = 1; i < fds.size(); ++i) {
            // Check for errors or hangup first
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                std::cout << "[DEBUG] Client fd " << fds[i].fd << " disconnected (POLLERR/POLLHUP/POLLNVAL)" << std::endl;
                close(fds[i].fd);
                clients.erase(fds[i].fd);
                fds.erase(fds.begin() + i);
                --i;
                continue;
            }

            // Handle outgoing data first
            if (fds[i].revents & POLLOUT) {
                std::cout << "[DEBUG] POLLOUT event for fd " << fds[i].fd << std::endl;
                Client& client = clients[fds[i].fd];
                if (client.hasDataToSend()) {
                    std::string& sendBuf = client.getSendBuffer();
                    ssize_t sent = send(fds[i].fd, sendBuf.c_str(), sendBuf.size(), 0);
                    std::cout << "[DEBUG] Attempting to send to fd " << fds[i].fd << ", buffer: '" << sendBuf << "', sent: " << sent << std::endl;
                    if (sent > 0) {
                        client.removeSentFromBuffer(sent);
                        if (!client.hasDataToSend()) {
                            fds[i].events = POLLIN; // Done sending, just listen for reads
                        }
                    } else if (sent < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
                        std::cerr << "send error on fd " << fds[i].fd << std::endl;
                        close(fds[i].fd);
                        clients.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                        continue;
                    }
                } else {
                    // No data to send, switch to only polling for input
                    fds[i].events = POLLIN;
                }
            }
            // Handle incoming data
            if (fds[i].revents & POLLIN) {
                char buf[512];
                ssize_t n = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);

                if (n > 0) {
                    buf[n] = '\0';
                    clients[fds[i].fd].appendToBuffer(std::string(buf, n));
                    clients[fds[i].fd].markReceivedData();
                    // Process complete messages here
                    std::cout << "Received data from " << fds[i].fd << ": " << std::string(buf, n) << std::endl;
                } else if (n == 0) {
                    // Connection closed by client - but only if they had sent data before
                    if (clients[fds[i].fd].hasReceivedData()) {
                        std::cout << "Client " << fds[i].fd << " disconnected." << std::endl;
                        close(fds[i].fd);
                        clients.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                        continue;
                    }
                    // If no data was ever received, don't disconnect - it's just a new idle client
                } else { // n < 0
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        std::cerr << "recv error on fd " << fds[i].fd << ": " << strerror(errno) << std::endl;
                        close(fds[i].fd);
                        clients.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                        continue;
                    }
                    // EAGAIN/EWOULDBLOCK means no data available right now, which is fine
                }
            }
        }
    }
}

