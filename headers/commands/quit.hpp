#pragma once

#include "../network/server.hpp"
#include "../network/client.hpp"
#include "../network/channel.hpp"
#include <string>
#include <vector>

class Server;
class Client;
class Channel;

class Quit {
public:
    static void execute(Server& server, Client& client, const std::vector<std::string>& params,
                       std::map<int, Client>& clients, std::vector<struct pollfd>& fds, int client_fd);

private:
    static void notifyChannelsOfQuit(Server& server, Client& client, const std::string& quitMessage);
    static void removeClientFromChannels(Server& server, Client& client);
    static void cleanupEmptyChannels(Server& server, Client& client);
};