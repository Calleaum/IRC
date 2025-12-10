#pragma once

/* -------------------------------------------------------------------------- */
/*                                 INCLUSIONS                                  */
/* -------------------------------------------------------------------------- */

#include "Bot.hpp"
#include "Client.hpp"
#include "IrcNumericReplies.hpp"
#include "IrcMessageBuilder.hpp"

#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stdint.h>
#include <map>

/* -------------------------------------------------------------------------- */
/*                                   CONSTANTES                                */
/* -------------------------------------------------------------------------- */

#define MAX_UINT16_BITS UINT16_MAX
#define IPV4 AF_INET
#define TCP SOCK_STREAM
#define DEFLT_PROT 0
#define GLOB_SOCK_OPT SOL_SOCKET
#define REUSE_ADDR SO_REUSEADDR
#define OPT_ON 1
#define ALL_ADDR INADDR_ANY
#define MAX_CONNEXIONS SOMAXCONN
#define FAILURE -1

#define TWO_ARGMNTS 2
#define THREE_ARGMNTS 3
#define FIVE_ARGMNTS 5

#define CTRL_C SIGINT
#define CTRL_Z SIGTSTP
#define PORT_ARG_INDEX 1

#define INVITE_ONLY 'i'
#define TOPIC_OP_ONLY 't'
#define KEY 'k'
#define OPERATOR 'o'
#define USER_LIMIT 'l'

#define IRC_BUFFER_SIZE 1024
#define LOCALHOST "127.0.0.1"

/* -------------------------------------------------------------------------- */
/*                              DÉCLARATIONS ANTICIPÉES                        */
/* -------------------------------------------------------------------------- */

class Client;
class Channel;

/* -------------------------------------------------------------------------- */
/*                                    CLASS SERVER                              */
/* -------------------------------------------------------------------------- */

class Server
{
public:

    static Server* instance;

    Server(unsigned short port, const std::string &password);
    ~Server();

    /* --------------------------- GESTION DU SERVEUR ------------------------- */
    void run();
    int getFdMax() const;
    fd_set& getMasterSet();
    const std::string& getServerName() const;
    void setFdMax(int fd);
    std::string& getServerIp();

    /* --------------------- INITIALISATION ET UTILITAIRES -------------------- */
    void init();
    void processCommand(Client *client, const std::string &message);
    void removeClient(Client *client);
    void registerClient(Client *client);
    void sendMotd(Client *client);
    std::vector<std::string> split(const std::string &str, const std::string &delim);
    Client* getClientByNickname(const std::string &nickname);
    Channel* getChannelFromMessage(const std::string &message);
    bool isValidNickname(const std::string &nickname);
    bool isValidUsername(const std::string &username);

    /* --------------------------- GESTION DES COMMANDES --------------------- */
    void handleNewConnection();
    void handleClientMessage(Client *client);

    void handlePassCommand(Client *client, const std::vector<std::string> &params);
    void handleNickCommand(Client *client, const std::vector<std::string> &params);
    void handleUserCommand(Client *client, const std::vector<std::string> &params);
    void handleJoinCommand(Client *client, const std::vector<std::string> &params);
    void handlePartCommand(Client *client, const std::vector<std::string> &params);
    void handlePrivmsgCommand(Client *client, const std::vector<std::string> &params);
    void handleModeCommand(Client *client, const std::vector<std::string> &params);
    void handleInviteCommand(Client *client, const std::vector<std::string> &params);
    void handleTopicCommand(Client *client, const std::vector<std::string> &params);
    void handleKickCommand(Client *client, const std::vector<std::string> &params);
    void handleCapCommand(Client *client, const std::vector<std::string> &params);
    bool handlePingPongCommand(Client *client, const std::string &args);

    /* -------------------- GESTION DES RÉPONSES ET MAINTENANCE ---------------- */
    void sendNamesReply(Client *client, Channel *channel);
    static void handleSignal(int signal);
    void shutdown();

    /* --------------------------- GESTION DU PING ---------------------------- */
    static const int PING_INTERVAL = 60;
    static const int PING_RESPONSE_DELAY = 15;

    bool send_message(const std::string &message, int sender_fd);
    std::string getBackgroundColorCode(int socket);
    void printClientInfo(int newSocket, const std::string& host);

private:

    /* ---------------------------- DONNÉES INTERNES ------------------------- */
    unsigned short _port;
    std::string _password;
    std::string _serverName;
    int _listenSocket;
    fd_set _masterSet;
    int _fdMax;
    std::map<std::string, Channel*> _channels;
    std::vector<Client*> _clients;
    Bot _bot;
    std::string _serverIp;
    std::vector<Client*> _clientsToRemove;

    std::string formatPingPongMessage(const std::string& client_id, const std::string& command, const std::string& param);
};
