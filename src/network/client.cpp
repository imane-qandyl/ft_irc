#include "client.hpp"

Client::Client() : _fd(-1), _receivedData(false) {
    std::cout << "Client default constructor called" << std::endl;
}

Client::Client(int fd) : _fd(fd), _receivedData(false) {
    std::cout << "Client constructor called for fd " << fd << std::endl;
}

// Copy constructor - don't close fd in the source object
Client::Client(const Client& other) 
    : _fd(other._fd), _receivedData(other._receivedData), 
      _buffer(other._buffer), _sendBuffer(other._sendBuffer),
      _nickname(other._nickname), _username(other._username),
      _channels(other._channels) {
    std::cout << "Client copy constructor called for fd " << _fd << std::endl;
}

// Assignment operator - don't close fd in the source object
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        // Don't close our current fd, let the server manage it
        _fd = other._fd;
        _receivedData = other._receivedData;
        _buffer = other._buffer;
        _sendBuffer = other._sendBuffer;
        _nickname = other._nickname;
        _username = other._username;
        _channels = other._channels;
        std::cout << "Client assignment operator called for fd " << _fd << std::endl;
    }
    return *this;
}

Client::~Client() {
    // Don't automatically close fd in destructor - let server manage it
    std::cout << "Client destructor called for fd " << _fd << std::endl;
}

void Client::markReceivedData() { _receivedData = true; }
bool Client::hasReceivedData() const { return _receivedData; }

int Client::getFd() const {
    return _fd;
}

std::string& Client::getBuffer() { return _buffer; }

void Client::appendToBuffer(const std::string& data) {
    size_t old_size = _buffer.size();
    _buffer += data;
    std::cout << "[DEBUG] Buffer for fd " << _fd << " grew from " << old_size << " to " << _buffer.size() << " bytes" << std::endl;
    
    // Prevent buffer overflow attacks
    if (_buffer.size() > 4096) {
        std::cout << "[WARNING] Buffer for fd " << _fd << " exceeded 4KB, truncating" << std::endl;
        _buffer = _buffer.substr(_buffer.size() - 2048); // Keep last 2KB
    }
}

bool Client::hasCompleteMessage() const {
    return _buffer.find("\r\n") != std::string::npos;
}

std::string Client::extractMessage() {
    size_t pos = _buffer.find("\r\n");
    if (pos == std::string::npos) {
        std::cout << "[DEBUG] No complete message in buffer for fd " << _fd << std::endl;
        return "";
    }
    
    std::string msg = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 2);
    std::cout << "[DEBUG] Extracted message from fd " << _fd << " buffer: '" << msg << "'" << std::endl;
    return msg;
}

void Client::appendToSendBuffer(const std::string& data) {
    size_t old_size = _sendBuffer.size();
    _sendBuffer += data;
    std::cout << "[DEBUG] Send buffer for fd " << _fd << " grew from " << old_size << " to " << _sendBuffer.size() << " bytes" << std::endl;
    
    // Prevent send buffer from growing too large
    if (_sendBuffer.size() > 8192) {
        std::cout << "[WARNING] Send buffer for fd " << _fd << " exceeded 8KB limit" << std::endl;
        // Could implement priority-based message dropping here
    }
}

bool Client::hasDataToSend() const {
    return !_sendBuffer.empty();
}

std::string& Client::getSendBuffer() {
    return _sendBuffer;
}

void Client::removeSentFromBuffer(size_t n) {
    if (n > _sendBuffer.size()) {
        std::cout << "[ERROR] Trying to remove " << n << " bytes from send buffer of size " << _sendBuffer.size() << std::endl;
        n = _sendBuffer.size();
    }
    
    _sendBuffer.erase(0, n);
    std::cout << "[DEBUG] Removed " << n << " bytes from send buffer for fd " << _fd << ", remaining: " << _sendBuffer.size() << std::endl;
}

void Client::setNickname(const std::string& nick) { _nickname = nick; }
const std::string& Client::getNickname() const { return _nickname; }

void Client::setUsername(const std::string& user) { _username = user; }
const std::string& Client::getUsername() const { return _username; }

void Client::joinChannel(const std::string& channel) { _channels.insert(channel); }
void Client::leaveChannel(const std::string& channel) { _channels.erase(channel); }
const std::set<std::string>& Client::getChannels() const { return _channels; }

