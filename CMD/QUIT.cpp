#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                              QUIT COMMAND                                  */
/* ************************************************************************** */

/**
 * @brief Recherche une sous-chaîne et extrait le reste de la commande
 * @param cmd Commande complète
 * @param tofind Mot-clé à rechercher
 * @param str Résultat : texte après le mot-clé
 */
void FindQ(std::string cmd, std::string tofind, std::string &str)
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
	// Extraction du texte restant (raison du quit)
	if (i < cmd.size())
		str = cmd.substr(i);
	// Suppression des espaces en début
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

/**
 * @brief Parse la commande QUIT et extrait la raison
 * @param cmd Commande brute
 * @return Raison du départ (préfixée par ':') ou "Quit" par défaut
 */
std::string	SplitQuit(std::string cmd)
{
	std::istringstream stm(cmd);
	std::string reason,str;
	stm >> str;
	FindQ(cmd, str, reason);
	// Raison par défaut si non spécifiée
	if (reason.empty())
		return std::string("Quit");
	// Si pas de ':', tronquer au premier espace et ajouter ':'
	if (reason[0] != ':')
	{
		for (size_t i = 0; i < reason.size(); i++)
		{
			if (reason[i] == ' ')
				{reason.erase(reason.begin() + i, reason.end());break;}
		}
		reason.insert(reason.begin(), ':');
	}
	return reason;
}

/**
 * @brief Commande QUIT - déconnecte le client du serveur
 * @param cmd Commande QUIT brute
 * @param fd File descriptor du client
 * 
 * Format : QUIT [:raison]
 * Notifie tous les canaux du départ et nettoie les ressources.
 */
void Server::QUIT(std::string cmd, int fd)
{
	std::string reason;
	reason = SplitQuit(cmd);
	// Parcours de tous les canaux pour notifier et retirer le client
	for (size_t i = 0; i < channels.size(); i++)
	{
		// Client est un membre normal du canal
		if (channels[i].get_client(fd)){
			channels[i].remove_client(fd);
			// Suppression du canal s'il est vide
			if (channels[i].GetClientsNumber() == 0)
				channels.erase(channels.begin() + i);
			else
			{
				// Notification aux autres membres
				std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT " + reason + "\r\n";
				channels[i].sendTo_all(rpl);
			}
		}
		// Client est un opérateur du canal
		else if (channels[i].get_admin(fd))
		{
			channels[i].remove_admin(fd);
			// Suppression du canal s'il est vide
			if (channels[i].GetClientsNumber() == 0)
				channels.erase(channels.begin() + i);
			else
			{
				// Notification aux autres membres
				std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT " + reason + "\r\n";
				channels[i].sendTo_all(rpl);
			}
		}
	}
	// Log de déconnexion et nettoyage des ressources
	std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
	RmChannels(fd);
	RemoveClient(fd);
	RemoveFds(fd);
	close(fd);
}