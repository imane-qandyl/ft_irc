#include "server.hpp"

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _server_fd(-1) {}

Server::~Server() {
    if (_server_fd != -1)
        close(_server_fd);
}

void Server::setupSocket() {
    // Use the setupServerSocket logic here
}

void Server::run() {
    // Main event loop (poll/select)
}