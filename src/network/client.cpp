#include "client.hpp"

Client::Client() : _fd(-1), _receivedData(false), _authenticated(false), _passwordProvided(false) {
    std::cout << "Client default constructor called" << std::endl;
}

Client::Client(int fd) : _fd(fd), _receivedData(false), _authenticated(false), _passwordProvided(false) {
    std::cout << "Client constructor called for fd " << fd << std::endl;
}

// Copy constructor - don't close fd in the source object
Client::Client(const Client& other) 
    : _fd(other._fd), _receivedData(other._receivedData), 
      _buffer(other._buffer), _sendBuffer(other._sendBuffer),
      _nickname(other._nickname), _username(other._username),
      _realname(other._realname), _hostname(other._hostname),
      _channels(other._channels), _authenticated(other._authenticated),
      _passwordProvided(other._passwordProvided) {
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
        _realname = other._realname;
        _hostname = other._hostname;
        _channels = other._channels;
        _authenticated = other._authenticated;
        _passwordProvided = other._passwordProvided;
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
    _buffer += data;
    
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
        return "";
    }
    
    std::string msg = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 2);
    return msg;
}

void Client::appendToSendBuffer(const std::string& data) {
    _sendBuffer += data;
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
}

void Client::setNickname(const std::string& nick) { _nickname = nick; }
const std::string& Client::getNickname() const { return _nickname; }

void Client::setUsername(const std::string& user) { _username = user; }
const std::string& Client::getUsername() const { return _username; }

void Client::joinChannel(const std::string& channel) { _channels.insert(channel); }
void Client::leaveChannel(const std::string& channel) { _channels.erase(channel); }
const std::set<std::string>& Client::getChannels() const { return _channels; }

// Channel management (pointer-based)
void Client::addChannel(Channel* channel) {
    if (channel) {
        _channelPointers.insert(channel);
        _channels.insert(channel->getName());
    }
}

void Client::removeChannel(Channel* channel) {
    if (channel) {
        _channelPointers.erase(channel);
        _channels.erase(channel->getName());

    }
}

// Authentication methods
bool Client::isAuthenticated() const {
    return _authenticated && _passwordProvided && !_nickname.empty() && !_username.empty();
}

void Client::setAuthenticated(bool auth) {
    _authenticated = auth;
}

// Password authentication
void Client::setPasswordProvided(bool provided) {
    _passwordProvided = provided;
}

bool Client::hasPasswordProvided() const {
    return _passwordProvided;
}

// Realname methods
void Client::setRealname(const std::string& realname) {
    _realname = realname;
}

const std::string& Client::getRealname() const {
    return _realname;
}

// Hostname methods
void Client::setHostname(const std::string& hostname) {
    _hostname = hostname;
}

const std::string& Client::getHostname() const {
    return _hostname;
}

// Message sending
void Client::sendMessage(const std::string& message) {
    appendToSendBuffer(message);
}