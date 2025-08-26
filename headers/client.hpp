#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <unistd.h>
#include <set>


class Client {
public:
    Client();
    Client(int fd);
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

    // Channels
    void joinChannel(const std::string& channel);
    void leaveChannel(const std::string& channel);
    const std::set<std::string>& getChannels() const;

private:
    int _fd;
    bool _receivedData; // Track if any data was received
    std::string _buffer; // For incoming data
    std::string _sendBuffer; // For outgoing data
    std::string _nickname;
    std::string _username;
    std::set<std::string> _channels;
};

#endif