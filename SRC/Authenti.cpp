#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                             PASS COMMAND                                   */
/* ************************************************************************** */

/**
 * @brief Authentifie un client avec le mot de passe serveur
 * @param fd File descriptor du client
 * @param cmd Commande PASS reçue avec le mot de passe
 * 
 * Vérifie que le mot de passe fourni correspond à celui du serveur.
 * Un client ne peut s'authentifier qu'une seule fois.
 */
void Server::client_authen(int fd, std::string cmd)
{
	Client *cli = GetClient(fd);
	// Supprime "PASS" de la commande pour extraire le mot de passe
	cmd = cmd.substr(4);
	// Ignore les espaces/tabulations en début de paramètre
	size_t pos = cmd.find_first_not_of("\t\v ");
	if(pos < cmd.size())
	{
		cmd = cmd.substr(pos);
		// Retire le ':' optionnel devant le mot de passe (syntaxe IRC)
		if(cmd[0] == ':')
			cmd.erase(cmd.begin());
	}
	// Vérifie qu'un mot de passe a été fourni
	if(pos == std::string::npos || cmd.empty()) 
		_sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), fd);
	// Client non encore authentifié : vérification du mot de passe
	else if(!cli->getRegistered())
	{
		std::string pass = cmd;
		if(pass == password)
			cli->setRegistered(true);  // Mot de passe correct
		else
            _sendResponse(ERR_INCORPASS(std::string("*")), fd);  // Mot de passe incorrect
	}
	// Client déjà authentifié : erreur
	else
        _sendResponse(ERR_ALREADYREGISTERED(GetClient(fd)->GetNickName()), fd);
}

/* ************************************************************************** */
/*                             NICK COMMAND                                   */
/* ************************************************************************** */

/**
 * @brief Vérifie la validité syntaxique d'un nickname
 * @param nickname Le nickname à valider
 * @return true si valide, false sinon
 * 
 * Règles : ne doit pas commencer par '&', '#' ou ':'
 * Caractères autorisés : alphanumériques et '_'
 */
bool Server::is_validNickname(std::string& nickname)
{
	// Le premier caractère ne doit pas être un caractère spécial
	if(!nickname.empty() && (nickname[0] == '&' || nickname[0] == '#' || nickname[0] == ':'))
		return false;
	// Les caractères suivants doivent être alphanumériques ou '_'
	for(size_t i = 1; i < nickname.size(); i++)
	{
		if(!std::isalnum(nickname[i]) && nickname[i] != '_')
			return false;
	}
	return true;
}


/**
 * @brief Vérifie si un nickname est déjà utilisé
 * @param nickname Le nickname à vérifier
 * @return true si le nickname est pris, false sinon
 */
bool Server::nickNameInUse(std::string& nickname)
{
	// Parcours de tous les clients connectés
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		// Collision détectée si un client a déjà ce nickname
		if (this->clients[i].GetNickName() == nickname)
			return true;
	}
	return false;
}


/**
 * @brief Définit ou modifie le nickname d'un client
 * @param cmd Commande NICK reçue
 * @param fd File descriptor du client
 * 
 * Gère l'attribution initiale du nickname ainsi que les changements.
 * Propage le changement à tous les canaux où le client est présent.
 */
void Server::set_nickname(std::string cmd, int fd)
{
	std::string inuse;
	// Supprime "NICK" de la commande
	cmd = cmd.substr(4);
	// Ignore les espaces/tabulations en début de paramètre
	size_t pos = cmd.find_first_not_of("\t\v ");
	if(pos < cmd.size())
	{
		cmd = cmd.substr(pos);
		// Retire le ':' optionnel (syntaxe IRC)
		if(cmd[0] == ':')
			cmd.erase(cmd.begin());
	}
	Client *cli = GetClient(fd);
	// Vérifie qu'un nickname a été fourni
	if(pos == std::string::npos || cmd.empty())
		{_sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), fd); return;}
	// Nickname déjà utilisé par un autre client
	if (nickNameInUse(cmd) && cli->GetNickName() != cmd){
		inuse = "*";
		// Assigne un placeholder si le client n'a pas encore de nick
		if(cli->GetNickName().empty())
			cli->SetNickname(inuse);
		_sendResponse(ERR_NICKINUSE(std::string(cmd)), fd); 
		return;
	}
	// Format du nickname invalide
	if(!is_validNickname(cmd)) {
		_sendResponse(ERR_ERRONEUSNICK(std::string(cmd)), fd);
		return;
	}
	else
	{
		// Client enregistré : mise à jour du nickname dans tous les canaux
		if(cli && cli->getRegistered())
		{
			std::string oldnick = cli->GetNickName();
			cli->SetNickname(cmd);
			// Propagation du changement dans tous les canaux
			for(size_t i = 0; i < channels.size(); i++)
			{
				Client *cl = channels[i].GetClientInChannel(oldnick);
				if(cl)
					cl->SetNickname(cmd);
			}
			// Notification du changement de nickname
			if(!oldnick.empty() && oldnick != cmd)
			{
				// Cas spécial : premier nick valide après placeholder "*"
				if(oldnick == "*" && !cli->GetUserName().empty())
				{
					cli->setLogedin(true);
					_sendResponse(RPL_CONNECTED(cli->GetNickName()), fd);
					_sendResponse(RPL_NICKCHANGE(cli->GetNickName(),cmd), fd);
				}
				else
					_sendResponse(RPL_NICKCHANGE(oldnick,cmd), fd);
				return;
			}

		}
		// Client non authentifié (PASS non validé)
		else if (cli && !cli->getRegistered())
			_sendResponse(ERR_NOTREGISTERED(cmd), fd);
	}
	// Connexion complète : PASS + NICK + USER validés
	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*" && !cli->GetLogedIn())
	{
		cli->setLogedin(true);
		_sendResponse(RPL_CONNECTED(cli->GetNickName()), fd);
	}
}

/* ************************************************************************** */
/*                             USER COMMAND                                   */
/* ************************************************************************** */

/**
 * @brief Définit le username d'un client
 * @param cmd Commande USER reçue (format: USER <username> <mode> <unused> <realname>)
 * @param fd File descriptor du client
 * 
 * Le username ne peut être défini qu'une seule fois.
 * Complete l'enregistrement si PASS et NICK sont déjà validés.
 */
void	Server::set_username(std::string& cmd, int fd)
{
	std::vector<std::string> splited_cmd = split_cmd(cmd);

	Client *cli = GetClient(fd); 
	if((cli && splited_cmd.size() < 5))
	{
		_sendResponse(ERR_NOTENOUGHPARAM(cli->GetNickName()), fd);
		return;
	}
	if(!cli  || !cli->getRegistered())
		_sendResponse(ERR_NOTREGISTERED(std::string("*")), fd);
	else if (cli && !cli->GetUserName().empty())
	{
		_sendResponse(ERR_ALREADYREGISTERED(cli->GetNickName()), fd);
		return;
	}
	else
		cli->SetUsername(splited_cmd[1]);
	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*"  && !cli->GetLogedIn())
	{
		cli->setLogedin(true);
		_sendResponse(RPL_CONNECTED(cli->GetNickName()), fd);
	}
}