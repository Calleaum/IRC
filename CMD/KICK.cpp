#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                              KICK COMMAND                                  */
/* ************************************************************************** */

/**
 * @brief Recherche une sous-chaîne et extrait le reste de la commande
 * @param cmd Commande complète
 * @param tofind Mot-clé à rechercher
 * @param str Résultat : texte après le mot-clé
 */
void FindK(std::string cmd, std::string tofind, std::string &str)
{
	size_t i = 0;
	// Recherche du mot-clé dans la commande
	for (; i < cmd.size(); i++){
		if (cmd[i] != ' '){
			std::string tmp;
			for (; i < cmd.size() && cmd[i] != ' '; i++)
				tmp += cmd[i];
			if (tmp == tofind) break;
			else tmp.clear();
		}
	}
	// Extraction du texte restant
	if (i < cmd.size()) str = cmd.substr(i);
	// Suppression des espaces en début
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

/**
 * @brief Parse les premiers éléments de la commande KICK
 * @param cmd Commande brute
 * @param tmp Vecteur de sortie pour les tokens
 * @return Raison du kick (si spécifiée)
 */
std::string SplitCmdK(std::string &cmd, std::vector<std::string> &tmp)
{
	std::stringstream ss(cmd);
	std::string str, reason;
	int count = 3;
	// Extraction des 3 premiers tokens : KICK #channel user
	while (ss >> str && count--)
		tmp.push_back(str);
	if(tmp.size() != 3) 
		return std::string("");
	// Extraction de la raison après le nom d'utilisateur
	FindK(cmd, tmp[2], reason);
	return reason;
}

/**
 * @brief Parse complètement la commande KICK
 * @param cmd Commande brute
 * @param tmp Vecteur de sortie pour les noms de canaux
 * @param user Nom de l'utilisateur à expulser (sortie)
 * @param fd File descriptor du client
 * @return Raison du kick
 * 
 * Format : KICK #chan1,#chan2 user [:raison]
 */
std::string Server::SplitCmdKick(std::string cmd, std::vector<std::string> &tmp, std::string &user, int fd)
{
	std::string reason = SplitCmdK(cmd, tmp);
	// Vérification des paramètres minimum
	if (tmp.size() < 3)
		return std::string("");
	tmp.erase(tmp.begin());  // Supprime "KICK"
	std::string str = tmp[0]; std::string str1;
	user = tmp[1]; tmp.clear();
	// Parsing des canaux séparés par ','
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == ',')
		{
			tmp.push_back(str1);
			str1.clear();
		}
		else str1 += str[i];
	}
	tmp.push_back(str1);
	// Nettoyage des entrées vides
	for (size_t i = 0; i < tmp.size(); i++)
	{
		if (tmp[i].empty())
			tmp.erase(tmp.begin() + i--);
	}
	// Traitement de la raison : supprime ':' ou tronque au premier espace
	if (reason[0] == ':') reason.erase(reason.begin());
	else
	{
		for (size_t i = 0; i < reason.size(); i++)
		{
			if (reason[i] == ' ')
			{
				reason = reason.substr(0, i);
				break;
			}
		}
	}
	// Validation des noms de canaux (doivent commencer par '#')
	for (size_t i = 0; i < tmp.size(); i++)
	{
		if (*(tmp[i].begin()) == '#')
			tmp[i].erase(tmp[i].begin());  // Supprime le '#'
		else
		{
			senderror(403, GetClient(fd)->GetNickName(), tmp[i], GetClient(fd)->GetFd(), " :No such channel\r\n"); tmp.erase(tmp.begin() + i--);
		}
	}
	return reason;
}

/**
 * @brief Commande KICK - expulse un utilisateur d'un ou plusieurs canaux
 * @param cmd Commande KICK brute
 * @param fd File descriptor de l'opérateur
 * 
 * Format : KICK #canal user [:raison]
 * Seuls les opérateurs du canal peuvent utiliser cette commande.
 */
void	Server::KICK(std::string cmd, int fd)
{
	std::vector<std::string> tmp;
	std::string reason ,user;
	// Parsing de la commande
	reason = SplitCmdKick(cmd, tmp, user, fd);
	// Vérification du paramètre utilisateur
	if (user.empty())
	{
		senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n");
		return;
	}
	// Traitement de chaque canal
	for (size_t i = 0; i < tmp.size(); i++)
	{
		// Vérification de l'existence du canal
		if (GetChannel(tmp[i]))
		{
			Channel *ch = GetChannel(tmp[i]);
			// Vérification que l'émetteur est sur le canal (ERR_NOTONCHANNEL 442)
			if (!ch->get_client(fd) && !ch->get_admin(fd))
			{
				senderror(442, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :You're not on that channel\r\n");
				continue;
			}
			// Vérification des privilèges opérateur (ERR_CHANOPRIVSNEEDED 482)
			if(ch->get_admin(fd))
			{
				// Vérification que la cible est sur le canal (ERR_USERNOTINCHANNEL 441)
				if (ch->GetClientInChannel(user))
				{
					// Construction du message KICK
					std::stringstream ss;
					ss << ":" << GetClient(fd)->GetNickName() << "!~" << GetClient(fd)->GetUserName() << "@" << "localhost" << " KICK #" << tmp[i] << " " << user;
					if (!reason.empty())
						ss << " :" << reason << "\r\n";
					else ss << "\r\n";
					// Notification à tous les membres du canal
					ch->sendTo_all(ss.str());
					// Suppression de la cible du canal
					if (ch->get_admin(ch->GetClientInChannel(user)->GetFd()))
						ch->remove_admin(ch->GetClientInChannel(user)->GetFd());
					else
						ch->remove_client(ch->GetClientInChannel(user)->GetFd());
					// Suppression du canal s'il est vide
					if (ch->GetClientsNumber() == 0)
						channels.erase(channels.begin() + i);
				}
				else
				{
					senderror(441, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :They aren't on that channel\r\n");
					continue;
				}
			}
			else
			{
				senderror(482, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :You're not channel operator\r\n");
				continue;
			}
		}
		else
			senderror(403, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :No such channel\r\n");
	}
}