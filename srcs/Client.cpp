/* Headers */
#include "../incs/Client.hpp"
#include "../incs/Channel.hpp"

/* Constructor */
Client::Client(int socket)
    : _socket(socket), _registered(false), _sentPass(false), _sentNick(false),
      _sentUser(false), _isAway(false), _isOperator(false), _lastPongTime(0),
      _lastActivityTime(time(NULL)), pingReceived(false)
{
    /* Sauvegarde l'heure d'activité */
    _lastActivityTime = time(NULL);
}

/* Destructor */
Client::~Client()
{
    close(_socket);
    for (std::set<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        (*it)->removeClient(this);
    _channels.clear();
}

/* Get socket */
int Client::getSocket() const
{ return _socket; }

/* Registered status */
bool Client::isRegistered() const
{ return _registered; }

void Client::setRegistered(bool status)
{ _registered = status; }

/* PASS status */
bool Client::hasSentPass() const
{ return _sentPass; }

void Client::setSentPass(bool status)
{ _sentPass = status; }

/* Buffer */
std::string& Client::getMessageBuffer()
{ return _messageBuffer; }

/* NICK status */
bool Client::hasSentNick() const
{ return _sentNick; }

void Client::setSentNick(bool status)
{ _sentNick = status; }

/* USER status */
bool Client::hasSentUser() const
{ return _sentUser; }

void Client::setSentUser(bool status)
{ _sentUser = status; }

/* Nickname */
const std::string &Client::getNickname() const
{ return _nickname; }

void Client::setNickname(const std::string &nickname)
{ _nickname = nickname; }

/* Username */
const std::string &Client::getUsername() const
{ return _username; }

void Client::setUsername(const std::string &username)
{ _username = username; }

/* Hostname */
const std::string &Client::getHostname() const
{ return _hostname; }
void Client::setHostname(const std::string &hostname)
{ _hostname = hostname; }

/* Servername */
const std::string &Client::getServername() const
{ return _servername; }

void Client::setServername(const std::string &servername)
{ _servername = servername; }

/* Realname */
const std::string &Client::getRealname() const
{ return _realname; }

void Client::setRealname(const std::string &realname)
{ _realname = realname; }

/* Join channel */
void Client::joinChannel(Channel *channel)
{ _channels.insert(channel); }

/* Leave channel */
void Client::leaveChannel(Channel *channel)
{ _channels.erase(channel); }

/* Check if in channel */
bool Client::isInChannel(const std::string &channelName) const
{
    for (std::set<Channel*>::const_iterator it = _channels.begin(); it != _channels.end(); ++it)
        if ((*it)->getName() == channelName)
            return true;
    return false;
}

/* Get channels */
const std::set<Channel*> &Client::getChannels() const
{ return _channels; }

/* PONG time */
void Client::setLastPongTime(time_t time)
{ _lastPongTime = time; }

time_t Client::getLastPongTime() const
{ return _lastPongTime; }

/* Away status */
bool Client::isAway() const
{ return _isAway; }

void Client::setAway(bool status)
{ _isAway = status; }

/* Operator status */
bool Client::isOperator() const
{ return _isOperator; }

void Client::setOperator(bool status)
{ _isOperator = status; }

/* Update activity time */
void Client::updateLastActivity()
{ _lastActivityTime = time(NULL); }

/* Ping flag */
bool Client::isPingReceived() const
{ return pingReceived; }

