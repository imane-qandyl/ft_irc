#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <iostream>


class Client;

class Channel {
private:
    std::string _name;
    std::set<Client*> _clients;

public:
    Channel(const std::string& name);
    ~Channel();
    
    const std::string& getName() const;
    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    size_t getClientCount() const;
    const std::set<Client*>& getClients() const;
};

#endif
