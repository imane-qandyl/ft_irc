#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <unistd.h>
#include <set>
#include <iostream>
#include "channel.hpp"
// Forward declaration
class Channel;

class Client {
public:
    Client();
    Client(int fd);
    Client(const Client& other);              // Copy constructor
    Client& operator=(const Client& other);   // Assignment operator
    ~Client();

    void markReceivedData();
    bool hasReceivedData() const;

    int getFd() const;
    std::string& getBuffer();
    void appendToBuffer(const std::string& data);
    bool hasCompleteMessage() const;
    std::string extractMessage();

    // Outgoing message buffer
    void appendToSendBuffer(const std::string& data);
    bool hasDataToSend() const;
    std::string& getSendBuffer();
    void removeSentFromBuffer(size_t n);

    // Nickname
    void setNickname(const std::string& nick);
    const std::string& getNickname() const;

    // Username
    void setUsername(const std::string& user);
    const std::string& getUsername() const;

    // Channels (string-based)
    void joinChannel(const std::string& channel);
    void leaveChannel(const std::string& channel);
    const std::set<std::string>& getChannels() const;
    
    // Channel management (pointer-based)
    void addChannel(Channel* channel);
    void removeChannel(Channel* channel);

    // Authentication
    bool isAuthenticated() const;
    void setAuthenticated(bool auth);
    
    // Password authentication
    void setPasswordProvided(bool provided);
    bool hasPasswordProvided() const;
    
    // Realname
    void setRealname(const std::string& realname);
    const std::string& getRealname() const;
    
    // Hostname
    void setHostname(const std::string& hostname);
    const std::string& getHostname() const;
    
    // Message sending
    void sendMessage(const std::string& message);

private:
    int _fd;
    bool _receivedData; // Track if any data was received
    std::string _buffer; // For incoming data
    std::string _sendBuffer; // For outgoing data
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;
    std::set<std::string> _channels;
    std::set<Channel*> _channelPointers; // Track channel pointers
    bool _authenticated;
    bool _passwordProvided;
};

#endif