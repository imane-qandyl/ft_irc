#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <iostream>
#include <ctime>

class Client;

class Channel {
private:
    std::string _name;
    std::set<Client*> _clients;
    std::set<Client*> _operators;
    std::set<Client*> _invited;
    std::string _key;
    std::string _topic;
    std::string _topicSetter;
    time_t _topicTime;
    bool _inviteOnly;
    bool _hasUserLimit;
    bool _topicRestricted;  // Add this line
    size_t _userLimit;

public:
    Channel(const std::string& name);
    ~Channel();
    
    // Basic channel operations
    const std::string& getName() const;
    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    size_t getClientCount() const;
    const std::set<Client*>& getClients() const;
    
    // Operator management
    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(Client* client) const;
    
    // Channel key/password
    bool hasKey() const;
    const std::string& getKey() const;
    void setKey(const std::string& key);
    void removeKey();
    
    // Invite-only mode
    bool isInviteOnly() const;
    void setInviteOnly(bool inviteOnly);
    bool isInvited(Client* client) const;
    void addInvite(Client* client);
    void removeInvite(Client* client);
    
    // User limit
    bool hasUserLimit() const;
    size_t getUserLimit() const;
    void setUserLimit(size_t limit);
    void removeUserLimit();
    
    // Topic management
    const std::string& getTopic() const;
    const std::string& getTopicSetter() const;
    time_t getTopicTime() const;
    void setTopic(const std::string& topic, const std::string& setter);
    bool hasTopicRestriction() const;
    void setTopicRestriction(bool restricted);
    
    // Broadcasting
    void broadcast(const std::string& message);
    void broadcast(const std::string& message, Client* exclude);

    bool isEmpty() const;
};

#endif