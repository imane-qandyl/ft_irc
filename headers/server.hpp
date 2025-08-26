#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include "server.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <poll.h>
#include <map>
#include "client.hpp"

class Server {

    private:
        int _port;
        std::string _password;
        int _server_fd;
        // Add containers for clients, channels, etc.
    public:
        Server(int port, const std::string& password);
        ~Server();

        void setupSocket();
        void run();

};

#endif