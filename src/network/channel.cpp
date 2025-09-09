#include "../headers/channel.hpp"
#include "../headers/client.hpp"
#include <iostream>

Channel::Channel(const std::string& name) : _name(name) {
    std::cout << "[INFO] Channel " << name << " created" << std::endl;
}

Channel::~Channel() {
    std::cout << "[INFO] Channel " << _name << " destroyed" << std::endl;
}

const std::string& Channel::getName() const {
    return _name;
}

void Channel::addClient(Client* client) {
    _clients.insert(client);
    std::cout << "[INFO] Client " << client->getNickname() << " added to channel " << _name << std::endl;
}

void Channel::removeClient(Client* client) {
    _clients.erase(client);
    std::cout << "[INFO] Client " << client->getNickname() << " removed from channel " << _name << std::endl;
}

bool Channel::hasClient(Client* client) const {
    return _clients.find(client) != _clients.end();
}

size_t Channel::getClientCount() const {
    return _clients.size();
}

const std::set<Client*>& Channel::getClients() const {
    return _clients;
}
