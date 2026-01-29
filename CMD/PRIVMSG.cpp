#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                            PRIVMSG COMMAND                                 */
/* ************************************************************************** */

/**
 * @brief Recherche une sous-chaîne et extrait le reste de la commande
 * @param cmd Commande complète
 * @param tofind Mot-clé à rechercher
 * @param str Résultat : texte après le mot-clé
 */
void FindPM(std::string cmd, std::string tofind, std::string &str)
{
	size_t i = 0;
	// Recherche du mot-clé dans la commande
	for (; i < cmd.size(); i++)
	{
		if (cmd[i] != ' ')
		{
			std::string tmp;
			for (; i < cmd.size() && cmd[i] != ' '; i++)
				tmp += cmd[i];
			if (tmp == tofind)
				break;
			else
				tmp.clear();
		}
	}
	// Extraction du texte restant (le message)
	if (i < cmd.size())
		str = cmd.substr(i);
	// Suppression des espaces en début
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

/**
 * @brief Parse les premiers éléments de la commande PRIVMSG
 * @param cmd Commande brute
 * @param tmp Vecteur de sortie pour les tokens
 * @return Message à envoyer
 */
std::string SplitCmdPM(std::string &cmd, std::vector<std::string> &tmp)
{
	std::stringstream ss(cmd);
	std::string str, msg;
	int count = 2;
	// Extraction des 2 premiers tokens : PRIVMSG <target>
	while (ss >> str && count--)
		tmp.push_back(str);
	if(tmp.size() != 2) return std::string("");
	// Extraction du message après la cible
	FindPM(cmd, tmp[1], msg);
	return msg;
}

/**
 * @brief Parse complètement la commande PRIVMSG
 * @param cmd Commande brute
 * @param tmp Vecteur de sortie pour les destinataires
 * @return Message à envoyer
 * 
 * Format : PRIVMSG #chan1,user1,#chan2 :message
 */
std::string SplitCmdPrivmsg(std::string cmd, std::vector<std::string> &tmp)
{
	std::string str = SplitCmdPM(cmd, tmp);
	// Vérification des paramètres minimum
	if (tmp.size() != 2)
	{
		tmp.clear();
		return std::string("");
	}
	tmp.erase(tmp.begin());  // Supprime "PRIVMSG"
	std::string str1 = tmp[0]; std::string str2; tmp.clear();
	// Parsing des destinataires séparés par ','
	for (size_t i = 0; i < str1.size(); i++)
	{
		if (str1[i] == ',')
		{
			tmp.push_back(str2);
			str2.clear();
		}
		else
			str2 += str1[i];
	}
	tmp.push_back(str2);
	// Nettoyage des entrées vides
	for (size_t i = 0; i < tmp.size(); i++)
	{
		if (tmp[i].empty())
			tmp.erase(tmp.begin() + i--);
	}
	// Traitement du message : supprime ':' ou tronque au premier espace
	if (str[0] == ':') str.erase(str.begin());
	else
	{
		for (size_t i = 0; i < str.size(); i++)
		{
				if (str[i] == ' ')
			{
				str = str.substr(0, i);
				break;
			}
		}
	}
	return  str;
}

/**
 * @brief Vérifie la validité des destinataires (canaux et utilisateurs)
 * @param tmp Vecteur des destinataires (modifié en place)
 * @param fd File descriptor de l'émetteur
 * 
 * Supprime les destinataires invalides et envoie les erreurs appropriées.
 */
void	Server::CheckForChannels_Clients(std::vector<std::string> &tmp, int fd)
{
	for(size_t i = 0; i < tmp.size(); i++)
	{
		// Destinataire est un canal (#channel)
		if (tmp[i][0] == '#')
		{
			tmp[i].erase(tmp[i].begin());  // Supprime le '#'
			// Canal inexistant (ERR_NOSUCHNICK 401)
			if(!GetChannel(tmp[i]))
			{
				senderror(401, "#" + tmp[i], GetClient(fd)->GetFd(), " :No such nick/channel\r\n");
				tmp.erase(tmp.begin() + i);
				i--;
			}
			// Émetteur pas sur le canal (ERR_CANNOTSENDTOCHAN 404)
			else if (!GetChannel(tmp[i])->GetClientInChannel(GetClient(fd)->GetNickName()))
			{
				senderror(404, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :Cannot send to channel\r\n");
				tmp.erase(tmp.begin() + i);
				i--;
			}
			else
				tmp[i] = "#" + tmp[i];  // Restaure le '#' pour identification
		}
		// Destinataire est un utilisateur
		else
		{
			// Utilisateur inexistant (ERR_NOSUCHNICK 401)
			if (!GetClientNick(tmp[i]))
			{
				senderror(401, tmp[i], GetClient(fd)->GetFd(), " :No such nick/channel\r\n");
				tmp.erase(tmp.begin() + i);
				i--;
			}
		}
	}
}

/**
 * @brief Commande PRIVMSG - envoie un message à des utilisateurs ou canaux
 * @param cmd Commande PRIVMSG brute
 * @param fd File descriptor de l'émetteur
 * 
 * Format : PRIVMSG <cible1,cible2,...> :<message>
 * Cibles : #canal pour un canal, nickname pour un utilisateur privé.
 */
void	Server::PRIVMSG(std::string cmd, int fd)
{
	std::vector<std::string> tmp;
	std::string message = SplitCmdPrivmsg(cmd, tmp);
	// Pas de destinataire spécifié (ERR_NORECIPIENT 411)
	if (!tmp.size())
	{
		senderror(411, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :No recipient given (PRIVMSG)\r\n");
		return;
	}
	// Pas de message spécifié (ERR_NOTEXTTOSEND 412)
	if (message.empty())
	{
		senderror(412, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :No text to send\r\n");
		return;
	}
	// Trop de destinataires (ERR_TOOMANYTARGETS 407)
	if (tmp.size() > 10)
	{
		senderror(407, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Too many recipients\r\n");
		return;
	}
	// Validation des destinataires
	CheckForChannels_Clients(tmp, fd);
	// Envoi du message à chaque destinataire valide
	for (size_t i = 0; i < tmp.size(); i++)
	{
		// Message vers un canal
		if (tmp[i][0] == '#')
		{
			tmp[i].erase(tmp[i].begin());
			std::string resp = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost PRIVMSG #" + tmp[i] + " :" + message + "\r\n";
			// Envoi à tous les membres sauf l'émetteur
			GetChannel(tmp[i])->sendTo_all(resp, fd);
		}
		// Message privé vers un utilisateur
		else
		{
			Client *target = GetClientNick(tmp[i]);
			if (target)
			{
				std::string resp = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost PRIVMSG " + target->GetNickName() + " :" + message + "\r\n";
				_sendResponse(resp, target->GetFd());
			}
		}
	}
}
