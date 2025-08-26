#include "../headers/server.hpp"

bool running = 1;

static void signal_handler(int signal)
{
    if (signal == SIGINT)
		running = 0;
    if (signal == SIGTSTP)
        running = 0;
    if (signal == SIGQUIT)
        running = 1;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
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
    Server server(port, password);
    server.setupSocket();
    server.run();
    std::cout << "Server is listening on port " << port << std::endl;

    // Continue with server setup...
    return 0;
}