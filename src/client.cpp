#include "../headers/client.hpp"

Client::Client() : _fd(-1), _receivedData(false) {} 
Client::Client(int fd) : _fd(fd), _receivedData(false) {}

Client::~Client() {
    if (_fd != -1)
        close(_fd);
}

void Client::markReceivedData() { _receivedData = true; }
bool Client::hasReceivedData() const { return _receivedData; }

int Client::getFd() const {
    return _fd;
}


std::string& Client::getBuffer() { return _buffer; }

void Client::appendToBuffer(const std::string& data) {
    _buffer += data;
}

bool Client::hasCompleteMessage() const {
    return _buffer.find("\r\n") != std::string::npos;
}

std::string Client::extractMessage() {
    size_t pos = _buffer.find("\r\n");
    if (pos == std::string::npos)
        return "";
    std::string msg = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 2); // Remove message and delimiter
    return msg;
}
void Client::appendToSendBuffer(const std::string& data) {
    _sendBuffer += data;
}

bool Client::hasDataToSend() const {
    return !_sendBuffer.empty();
}

std::string& Client::getSendBuffer() {
    return _sendBuffer;
}

void Client::removeSentFromBuffer(size_t n) {
    _sendBuffer.erase(0, n);
}
void Client::setNickname(const std::string& nick) { _nickname = nick; }
const std::string& Client::getNickname() const { return _nickname; }

void Client::setUsername(const std::string& user) { _username = user; }
const std::string& Client::getUsername() const { return _username; }

void Client::joinChannel(const std::string& channel) { _channels.insert(channel); }
void Client::leaveChannel(const std::string& channel) { _channels.erase(channel); }
const std::set<std::string>& Client::getChannels() const { return _channels; }

