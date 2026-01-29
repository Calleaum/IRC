#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                              MODE COMMAND                                  */
/* ************************************************************************** */

/**
 * @brief Construit la chaîne de modes à ajouter
 * @param chain Chaîne de modes existante
 * @param opera Opérateur ('+' ou '-')
 * @param mode Caractère du mode à ajouter
 * @return Chaîne formatée à ajouter (ex: "+i" ou "i" si même opérateur)
 */
std::string Server::mode_toAppend(std::string chain, char opera, char mode)
{
	std::stringstream ss;

	ss.clear();
	// Recherche du dernier opérateur dans la chaîne
	char last = '\0';
	for(size_t i = 0; i < chain.size(); i++)
	{
		if(chain[i] == '+' || chain[i] == '-')
			last = chain[i];
	}
	// Ajoute l'opérateur seulement s'il diffère du précédent
	if(last != opera)
		ss << opera << mode;
	else
		ss << mode;
	return ss.str();
}

/**
 * @brief Extrait les arguments de la commande MODE
 * @param cmd Commande brute
 * @param name Nom du canal (sortie)
 * @param modeset Chaîne de modes (sortie, ex: "+it-l")
 * @param params Paramètres des modes (sortie)
 */
void Server::getCmdArgs(std::string cmd,std::string& name, std::string& modeset ,std::string &params)
{
	std::istringstream stm(cmd);
	stm >> name;
	stm >> modeset;

	// Lecture du reste de la ligne pour préserver les espaces dans les paramètres
	std::string rest;
	std::getline(stm, rest);
	// Suppression des espaces en début
	size_t found = rest.find_first_not_of(" \t\v");
	if (found != std::string::npos)
		params = rest.substr(found);
	else
		params.clear();
}


/**
 * @brief Découpe les paramètres de mode séparés par des virgules
 * @param params Chaîne de paramètres (ex: "pass1,user1,10")
 * @return Vecteur de paramètres individuels
 */
std::vector<std::string> Server::splitParams(std::string params)
{
	// Suppression du ':' initial si présent
	if(!params.empty() && params[0] == ':')
		params.erase(params.begin());
	std::vector<std::string> tokens;
	std::string param;
	std::istringstream stm(params);
	// Découpage par virgules
	while (std::getline(stm, param, ','))
	{
		tokens.push_back(param);
		param.clear();
	}
	return tokens;
}

/**
 * @brief Commande MODE - gère les modes d'un canal
 * @param cmd Commande MODE brute
 * @param fd File descriptor de l'opérateur
 * 
 * Modes supportés :
 *   +i : canal invite-only
 *   +t : seuls les ops peuvent changer le topic
 *   +k : mot de passe requis pour rejoindre
 *   +o : donne/retire les privilèges opérateur
 *   +l : limite le nombre de membres
 */
void Server::mode_command(std::string& cmd, int fd)
{
	std::string channelName;
	std::string params;
	std::string modeset;
	std::stringstream mode_chain;
	std::string arguments = ":";
	Channel *channel;
	char opera = '\0';

	arguments.clear();
	mode_chain.clear();
	Client *cli = GetClient(fd);
	// Extraction du contenu après "MODE"
	size_t found = cmd.find_first_not_of("MODEmode \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	else
	{
		_sendResponse(ERR_NOTENOUGHPARAM(cli->GetNickName()), fd); 
		return ;
	}
	getCmdArgs(cmd ,channelName, modeset ,params);
	std::vector<std::string> tokens = splitParams(params);
	// Requête de mode utilisateur (pas un canal) - réponse vide pour compatibilité irssi
	if(channelName[0] != '#')
	{
		_sendResponse(":localhost 221 " + cli->GetNickName() + " +\r\n", fd);
		return ;
	}
	// Vérification de l'existence du canal
	if(!(channel = GetChannel(channelName.substr(1))))
	{
		_sendResponse(ERR_CHANNELNOTFOUND(cli->GetUserName(),channelName), fd);
		return ;
	}
	// Vérification que le client est sur le canal
	else if (!channel->get_client(fd) && !channel->get_admin(fd))
	{
		senderror(442, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->GetFd(), " :You're not on that channel\r\n");
		return;
	}
	// Sans modeset : retourne les modes actuels du canal
	else if (modeset.empty())
	{
		_sendResponse(RPL_CHANNELMODES(cli->GetNickName(), channel->GetName(), channel->getModes()) + \
		RPL_CREATIONTIME(cli->GetNickName(), channel->GetName(),channel->get_creationtime()),fd);
		return ;
	}
	// Vérification des privilèges opérateur
	else if (!channel->get_admin(fd))
	{
		_sendResponse(ERR_NOTOPERATOR(channel->GetName()), fd);
		return ;
	}
	// Traitement de chaque caractère de mode
	else if(channel)
	{
		size_t pos = 0;
		for(size_t i = 0; i < modeset.size(); i++)
		{
			// Détection de l'opérateur (+/-)
			if(modeset[i] == '+' || modeset[i] == '-')
				opera = modeset[i];
			else
			{
				// Dispatch selon le mode demandé
				if(modeset[i] == 'i')  // Mode invite-only
					mode_chain << invite_only(channel , opera, mode_chain.str());
				else if (modeset[i] == 't')  // Mode topic restricted
					mode_chain << topic_restriction(channel, opera, mode_chain.str());
				else if (modeset[i] == 'k')  // Mode key (mot de passe)
					mode_chain <<  password_mode(tokens, channel, pos, opera, fd, mode_chain.str(), arguments);
				else if (modeset[i] == 'o')  // Mode operator
						mode_chain << operator_privilege(tokens, channel, pos, fd, opera, mode_chain.str(), arguments);
				else if (modeset[i] == 'l')  // Mode limit
					mode_chain << channel_limit(tokens, channel, pos, opera, fd, mode_chain.str(), arguments);
				else
					_sendResponse(ERR_UNKNOWNMODE(cli->GetNickName(), channel->GetName(),modeset[i]),fd);
			}
		}
	}
	// Diffusion des changements de mode si nécessaire
	std::string chain = mode_chain.str();
	if(chain.empty())
		return ;
 	channel->sendTo_all(RPL_CHANGEMODE(cli->getHostname(), channel->GetName(), mode_chain.str(), arguments));
}

/**
 * @brief Gère le mode +i/-i (invite-only)
 * @param channel Canal à modifier
 * @param opera Opérateur (+/-)
 * @param chain Chaîne de modes actuelle
 * @return Chaîne à ajouter à la réponse
 */
std::string Server::invite_only(Channel *channel, char opera, std::string chain)
{
	std::string param;
	param.clear();
	// Activation du mode invite-only
	if(opera == '+' && !channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, true);
		channel->SetInvitOnly(1);
		param =  mode_toAppend(chain, opera, 'i');
	}
	// Désactivation du mode invite-only
	else if (opera == '-' && channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, false);
		channel->SetInvitOnly(0);
		param =  mode_toAppend(chain, opera, 'i');
	}
	return param;
}

/**
 * @brief Gère le mode +t/-t (topic restriction)
 * @param channel Canal à modifier
 * @param opera Opérateur (+/-)
 * @param chain Chaîne de modes actuelle
 * @return Chaîne à ajouter à la réponse
 */
std::string Server::topic_restriction(Channel *channel ,char opera, std::string chain)
{
	std::string param;
	param.clear();
	// Activation : seuls les ops peuvent changer le topic
	if(opera == '+' && !channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, true);
		channel->set_topicRestriction(true);
		param =  mode_toAppend(chain, opera, 't');
	}
	// Désactivation : tout le monde peut changer le topic
	else if (opera == '-' && channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, false);
		channel->set_topicRestriction(false);
		param =  mode_toAppend(chain, opera, 't');
	}	
	return param;
}

/**
 * @brief Valide le format d'un mot de passe de canal
 * @param password Mot de passe à valider
 * @return true si valide (alphanumérique + '_'), false sinon
 */
bool validPassword(std::string password)
{
	if(password.empty())
		return false;
	for(size_t i = 0; i < password.size(); i++)
	{
		if(!std::isalnum(password[i]) && password[i] != '_')
			return false;
	}
	return true;
}

/**
 * @brief Gère le mode +k/-k (mot de passe du canal)
 * @param tokens Paramètres de la commande
 * @param channel Canal à modifier
 * @param pos Position actuelle dans tokens
 * @param opera Opérateur (+/-)
 * @param fd File descriptor du client
 * @param chain Chaîne de modes actuelle
 * @param arguments Arguments à ajouter à la réponse
 * @return Chaîne à ajouter à la réponse
 */
std::string Server::password_mode(std::vector<std::string> tokens, Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string &arguments)
{
	std::string param;
	std::string pass;

	param.clear();
	pass.clear();
	// Extraction du mot de passe depuis les paramètres
	if(tokens.size() > pos)
		pass = tokens[pos++];
	else
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(),std::string("(k)")),fd);
		return param;
	}
	// Validation du format du mot de passe
	if(!validPassword(pass))
	{
		_sendResponse(ERR_INVALIDMODEPARM(channel->GetName(),std::string("(k)")),fd);
		return param;
	}
	// Activation : définit le mot de passe du canal
	if(opera == '+')
	{
		channel->setModeAtindex(2, true);
		channel->SetPassword(pass);
		if(!arguments.empty())
			arguments += " ";
		arguments += pass;
		param = mode_toAppend(chain,opera, 'k');
	}
	// Désactivation : supprime le mot de passe (nécessite le mot de passe actuel)
	else if (opera == '-' && channel->getModeAtindex(2))
	{
		if(pass == channel->GetPassword())
		{		
			channel->setModeAtindex(2, false);
			channel->SetPassword("");
			param = mode_toAppend(chain,opera, 'k');
		}
		else
			_sendResponse(ERR_KEYSET(channel->GetName()),fd);
	}
	return param;
}

/**
 * @brief Gère le mode +o/-o (privilèges opérateur)
 * @param tokens Paramètres de la commande
 * @param channel Canal à modifier
 * @param pos Position actuelle dans tokens
 * @param fd File descriptor du client
 * @param opera Opérateur (+/-)
 * @param chain Chaîne de modes actuelle
 * @param arguments Arguments à ajouter à la réponse
 * @return Chaîne à ajouter à la réponse
 */
std::string Server::operator_privilege(std::vector<std::string> tokens, Channel *channel, size_t& pos, int fd, char opera, std::string chain, std::string& arguments)
{
	std::string user;
	std::string param;

	param.clear();
	user.clear();
	// Extraction du nickname depuis les paramètres
	if(tokens.size() > pos)
		user = tokens[pos++];
	else
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(),"(o)"),fd);
		return param;
	}
	// Vérification que l'utilisateur cible est sur le canal
	if(!channel->clientInChannel(user))
	{
		_sendResponse(ERR_NOSUCHNICK(channel->GetName(), user),fd);
		return param;
	}
	// Promotion : client -> admin (opérateur)
	if(opera == '+')
	{
		channel->setModeAtindex(3,true);
		if(channel->change_clientToAdmin(user))
		{
			param = mode_toAppend(chain, opera, 'o');
			if(!arguments.empty())
				arguments += " ";
			arguments += user;
		}
	}
	// Rétrogradation : admin -> client (retrait des droits op)
	else if (opera == '-')
	{
		channel->setModeAtindex(3,false);
		if(channel->change_adminToClient(user))
		{
			param = mode_toAppend(chain, opera, 'o');
				if(!arguments.empty())
					arguments += " ";
			arguments += user;

		}
	}
	return param;
}

/**
 * @brief Valide le format d'une limite de membres
 * @param limit Chaîne à valider
 * @return true si entier positif valide, false sinon
 */
bool Server::isvalid_limit(std::string& limit)
{
	return (!(limit.find_first_not_of("0123456789")!= std::string::npos) && std::atoi(limit.c_str()) > 0);
}

/**
 * @brief Gère le mode +l/-l (limite de membres)
 * @param tokens Paramètres de la commande
 * @param channel Canal à modifier
 * @param pos Position actuelle dans tokens
 * @param opera Opérateur (+/-)
 * @param fd File descriptor du client
 * @param chain Chaîne de modes actuelle
 * @param arguments Arguments à ajouter à la réponse
 * @return Chaîne à ajouter à la réponse
 */
std::string Server::channel_limit(std::vector<std::string> tokens,  Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string& arguments)
{
	std::string limit;
	std::string param;

	param.clear();
	limit.clear();
	// Activation : définit une limite de membres
	if(opera == '+')
	{
		if(tokens.size() > pos )
		{
			limit = tokens[pos++];
			// Validation de la valeur
			if(!isvalid_limit(limit))
			{
				_sendResponse(ERR_INVALIDMODEPARM(channel->GetName(),"(l)"), fd);
			}
			else
			{
				channel->setModeAtindex(4, true);
				channel->SetLimit(std::atoi(limit.c_str()));
				if(!arguments.empty())
					arguments += " ";
				arguments += limit;
				param =  mode_toAppend(chain, opera, 'l');
			}
		}
		else
			_sendResponse(ERR_NEEDMODEPARM(channel->GetName(),"(l)"),fd);
	}
	// Désactivation : supprime la limite
	else if (opera == '-' && channel->getModeAtindex(4))
	{
		channel->setModeAtindex(4, false);
		channel->SetLimit(0);
		param  = mode_toAppend(chain, opera, 'l');
	}
	return param;
}