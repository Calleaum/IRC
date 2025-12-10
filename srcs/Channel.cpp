/* Headers */
#include "../incs/Channel.hpp"
#include "../incs/Client.hpp"

/* Constructor */
Channel::Channel(const std::string &name)
: _name(name), _userLimit(0), _hasTopic(false)
{}

/* Destructor */
Channel::~Channel()
{
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        (*it)->leaveChannel(this);
    _clients.clear();
}

/* Get channel name */
const std::string &Channel::getName() const { return _name; }

/* Add client */
void Channel::addClient(Client *client)
{
    if (!hasClient(client))
        _clients.push_back(client);
}

/* Remove client */
void Channel::removeClient(Client *client)
{
    _clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
    _operators.erase(client);
    _invitedClients.erase(client);
}

/* Check if client is in channel */
bool Channel::hasClient(Client *client) const
{
    return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

/* Get all clients */
const std::vector<Client*> &Channel::getClients() const { return _clients; }

/* Set/unset a mode */
void Channel::setMode(char mode) { _modes.insert(mode); }
void Channel::unsetMode(char mode) { _modes.erase(mode); }
bool Channel::hasMode(char mode) const { return _modes.find(mode) != _modes.end(); }

/* Set/unset channel key */
void Channel::setKey(const std::string &key) { _key = key; setMode('k'); }
void Channel::unsetKey() { _key.clear(); unsetMode('k'); }
bool Channel::checkKey(const std::string &key) const { return _key == key; }
bool Channel::hasKey() const { return hasMode('k'); }
const std::string &Channel::getKey() const { return _key; }

/* Set/unset user limit */
void Channel::setUserLimit(int limit) { _userLimit = limit; setMode('l'); }
void Channel::unsetUserLimit() { _userLimit = 0; unsetMode('l'); }
int Channel::getUserLimit() const { return _userLimit; }

/* Check if channel is full */
bool Channel::isFull() const { return hasMode('l') && static_cast<int>(_clients.size()) >= _userLimit; }

/* Operator management */
void Channel::addOperator(Client *client) { _operators.insert(client); }
void Channel::removeOperator(Client *client) { _operators.erase(client); }
bool Channel::isOperator(Client *client) const { return _operators.find(client) != _operators.end(); }

/* Invite management */
void Channel::inviteClient(Client *client) { _invitedClients.insert(client); }
bool Channel::isInvited(Client *client) const { return _invitedClients.find(client) != _invitedClients.end(); }
void Channel::removeInvitation(Client *client) { _invitedClients.erase(client); }

/* Topic management */
void Channel::setTopic(const std::string &topic) { _topic = topic; _hasTopic = true; }
const std::string &Channel::getTopic() const { return _topic; }
bool Channel::hasTopic() const { return _hasTopic; }

/* Check if client has voice */
bool Channel::hasVoice(const Client *client) const
{
    return _voicedClients.find(const_cast<Client*>(client)) != _voicedClients.end();
}
