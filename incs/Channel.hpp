#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>

/* Déclaration anticipée de Client */
class Client;

class Channel
{
public:
    /* Constructeur / destructeur */
    Channel(const std::string &name);
    ~Channel();

    /* Nom du canal */
    const std::string &getName() const;

    /* ----- CLIENT ----- */
    void addClient(Client *client);       // Ajouter un client
    void removeClient(Client *client);    // Retirer un client
    bool hasClient(Client *client) const; // Vérifier si client est dans le canal
    const std::vector<Client*> &getClients() const; // Liste des clients

    /* ----- MODE ----- */
    void setMode(char mode);    // Ajouter un mode
    void unsetMode(char mode);  // Retirer un mode
    bool hasMode(char mode) const; // Vérifier si mode est actif

    /* ----- KEY ----- */
    void setKey(const std::string &key); // Définir clé
    void unsetKey();                     // Retirer clé
    bool checkKey(const std::string &key) const; // Vérifier clé
    bool hasKey() const;                 // Vérifier si canal a une clé
    const std::string &getKey() const;

    /* ----- LIMIT ----- */
    void setUserLimit(int limit);  // Définir limite clients
    void unsetUserLimit();         // Retirer limite
    int getUserLimit() const;      // Obtenir limite
    bool isFull() const;           // Vérifier si canal plein

    /* ----- OPERATOR ----- */
    void addOperator(Client *client);    // Ajouter opérateur
    void removeOperator(Client *client); // Retirer opérateur
    bool isOperator(Client *client) const; // Vérifier opérateur

    /* ----- INVITE ----- */
    void inviteClient(Client *client);    // Inviter client
    bool isInvited(Client *client) const; // Vérifier invitation
    void removeInvitation(Client *client);// Retirer invitation

    /* ----- TOPIC ----- */
    void setTopic(const std::string &topic);   // Définir sujet
    const std::string &getTopic() const;       // Obtenir sujet
    bool hasTopic() const;                      // Vérifier si sujet
    bool hasVoice(const Client *client) const; // Vérifier si client a voix

private:
    std::string _name;                 // Nom canal
    std::vector<Client*> _clients;     // Clients
    std::set<char> _modes;             // Modes
    std::string _key;                   // Clé
    int _userLimit;                     // Limite clients
    std::set<Client*> _operators;       // Opérateurs
    std::set<Client*> _invitedClients;  // Clients invités
    std::string _topic;                 // Sujet
    bool _hasTopic;                     // Sujet défini ?
    std::set<Client*> _voicedClients;   // Clients avec voix
};

#endif /* CHANNEL_HPP */
