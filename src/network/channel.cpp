#include "channel.hpp"
#include "client.hpp"

Channel::Channel(const std::string& name) 
    : _name(name), _topicTime(0), _inviteOnly(false), _hasUserLimit(false), _topicRestricted(true), _userLimit(0) {
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
    _operators.erase(client);
    _invited.erase(client);
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

// Operator management
void Channel::addOperator(Client* client) {
    _operators.insert(client);
    std::cout << "[INFO] Client " << client->getNickname() << " is now operator of " << _name << std::endl;
}

void Channel::removeOperator(Client* client) {
    _operators.erase(client);
    std::cout << "[INFO] Client " << client->getNickname() << " is no longer operator of " << _name << std::endl;
}

bool Channel::isOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}

// Channel key/password
bool Channel::hasKey() const {
    return !_key.empty();
}

const std::string& Channel::getKey() const {
    return _key;
}

void Channel::setKey(const std::string& key) {
    _key = key;
    std::cout << "[INFO] Channel " << _name << " key set" << std::endl;
}

void Channel::removeKey() {
    _key.clear();
    std::cout << "[INFO] Channel " << _name << " key removed" << std::endl;
}

// Invite-only mode
bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

void Channel::setInviteOnly(bool inviteOnly) {
    _inviteOnly = inviteOnly;
    std::cout << "[INFO] Channel " << _name << " invite-only mode " << (inviteOnly ? "enabled" : "disabled") << std::endl;
}

bool Channel::isInvited(Client* client) const {
    return _invited.find(client) != _invited.end();
}

void Channel::addInvite(Client* client) {
    _invited.insert(client);
    std::cout << "[INFO] Client " << client->getNickname() << " invited to " << _name << std::endl;
}

void Channel::removeInvite(Client* client) {
    _invited.erase(client);
    std::cout << "[INFO] Invite for " << client->getNickname() << " to " << _name << " removed" << std::endl;
}

// User limit
bool Channel::hasUserLimit() const {
    return _hasUserLimit;
}

size_t Channel::getUserLimit() const {
    return _userLimit;
}

void Channel::setUserLimit(size_t limit) {
    _userLimit = limit;
    _hasUserLimit = true;
    std::cout << "[INFO] Channel " << _name << " user limit set to " << limit << std::endl;
}

void Channel::removeUserLimit() {
    _hasUserLimit = false;
    _userLimit = 0;
    std::cout << "[INFO] Channel " << _name << " user limit removed" << std::endl;
}

// Topic management
const std::string& Channel::getTopic() const {
    return _topic;
}

const std::string& Channel::getTopicSetter() const {
    return _topicSetter;
}

time_t Channel::getTopicTime() const {
    return _topicTime;
}

void Channel::setTopic(const std::string& topic, const std::string& setter) {
    _topic = topic;
    _topicSetter = setter;
    _topicTime = time(NULL);
    std::cout << "[INFO] Channel " << _name << " topic set by " << setter << std::endl;
}

// Broadcasting
void Channel::broadcast(const std::string& message) {
    broadcast(message, NULL);
}

void Channel::broadcast(const std::string& message, Client* exclude) {
    for (std::set<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it != exclude) {
            (*it)->sendMessage(message);
        }
    }
}

bool Channel::hasTopicRestriction() const {
    return _topicRestricted;
}

void Channel::setTopicRestriction(bool restricted) {
    _topicRestricted = restricted;
    std::cout << "[INFO] Channel " << _name << " topic restriction " << (restricted ? "enabled" : "disabled") << std::endl;
}

bool Channel::isEmpty() const {
    return _clients.empty();
}