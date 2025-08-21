#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    std::istringstream portStream(argv[1]);
    int port;
    if (!(portStream >> port) || port < 1024 || port > 65535) {
        std::cerr << "Error: Port must be a number between 1024 and 65535." << std::endl;
        return 1;
    }

    std::string password = argv[2];
    if (password.empty()) {
        std::cerr << "Error: Password cannot be empty." << std::endl;
        return 1;
    }

    // Continue with server setup...
    return 0;
}