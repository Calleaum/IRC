#pragma once

#include <string>
#include <set>
#include <ctime>
#include <unistd.h> // pour close()

class Channel;

class Client
{
public:
    /* Constructeur / destructeur */
    Client(int socket);
    ~Client();

    /* Socket */
    int getSocket() const;

    /* ----- ENREGISTREMENT ----- */
    bool isRegistered() const;
    void setRegistered(bool status);

    bool hasSentPass() const;
    void setSentPass(bool status);

    bool hasSentNick() const;
    void setSentNick(bool status);

    bool hasSentUser() const;
    void setSentUser(bool status);

    /* ----- STATUT ----- */
    bool isAway() const;
    void setAway(bool status);

    bool isOperator() const;
    void setOperator(bool status);

    void updateLastActivity(); // Met à jour l'heure de dernière activité

    /* ----- IDENTITÉ ----- */
    const std::string &getNickname() const;
    void setNickname(const std::string &nickname);

    const std::string &getUsername() const;
    void setUsername(const std::string &username);

    const std::string &getHostname() const;
    void setHostname(const std::string &hostname);

    const std::string &getServername() const;
    void setServername(const std::string &servername);

    const std::string &getRealname() const;
    void setRealname(const std::string &realname);

    /* ----- CANAUX ----- */
    void joinChannel(Channel *channel);
    void leaveChannel(Channel *channel);
    bool isInChannel(const std::string &channelName) const;
    const std::set<Channel*> &getChannels() const;

    /* ----- PONG / MESSAGES ----- */
    void setLastPongTime(time_t time);
    time_t getLastPongTime() const;

    std::string &getMessageBuffer();
    bool isPingReceived() const;

private:
    /* Socket du client */
    int _socket;

    /* Identité */
    std::string _nickname;
    std::string _username;
    std::string _hostname;
    std::string _servername;
    std::string _realname;

    std::string _messageBuffer;

    /* États d'enregistrement */
    bool _registered;
    bool _sentPass;
    bool _sentNick;
    bool _sentUser;

    bool _isAway;
    bool _isOperator;

    /* Canaux */
    std::set<Channel*> _channels;

    /* Ping / activité */
    time_t _lastPongTime;
    time_t _lastActivityTime;
    bool pingReceived;
};
