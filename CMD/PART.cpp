#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                              PART COMMAND                                  */
/* ************************************************************************** */

/**
 * @brief Recherche une sous-chaîne et extrait le reste de la commande
 * @param cmd Commande complète
 * @param tofind Mot-clé à rechercher
 * @param str Résultat : texte après le mot-clé
 */
void FindPR(std::string cmd, std::string tofind, std::string &str)
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
	// Extraction du texte restant
	if (i < cmd.size())
		str = cmd.substr(i);
	// Suppression des espaces en début
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

/**
 * @brief Parse les premiers éléments de la commande PART
 * @param cmd Commande brute
 * @param tmp Vecteur de sortie pour les tokens
 * @return Raison du départ (si spécifiée)
 */
std::string SplitCmdPR(std::string &cmd, std::vector<std::string> &tmp)
{
	std::stringstream ss(cmd);
	std::string str, reason;
	int count = 2;
	// Extraction des 2 premiers tokens : PART #channel
	while (ss >> str && count--)
		tmp.push_back(str);
	if(tmp.size() != 2)
		return std::string("");
	// Extraction de la raison après le nom du canal
	FindPR(cmd, tmp[1], reason);
	return reason;
}

/**
 * @brief Parse complètement la commande PART
 * @param cmd Commande brute
 * @param tmp Vecteur de sortie pour les noms de canaux
 * @param reason Raison du départ (sortie)
 * @param fd File descriptor du client
 * @return 1 si parsing réussi, 0 si erreur
 * 
 * Format : PART #chan1,#chan2 [:raison]
 */
int Server::SplitCmdPart(std::string cmd, std::vector<std::string> &tmp, std::string &reason, int fd)
{
	reason = SplitCmdPR(cmd, tmp);
	// Vérification des paramètres minimum
	if(tmp.size() < 2) 
	{
		tmp.clear();
		return 0;
	}
	tmp.erase(tmp.begin());  // Supprime "PART"
	std::string str = tmp[0]; std::string str1; tmp.clear();
	// Parsing des canaux séparés par ','
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == ',')
		{
			tmp.push_back(str1);
			str1.clear();
		}
		else
			str1 += str[i];
	}
	tmp.push_back(str1);
	// Nettoyage des entrées vides
	for (size_t i = 0; i < tmp.size(); i++)
	{
		if (tmp[i].empty())
			tmp.erase(tmp.begin() + i--);
	}
	// Traitement de la raison
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
	// Validation des noms de canaux
	for (size_t i = 0; i < tmp.size(); i++)
	{
		if (*(tmp[i].begin()) == '#')
			tmp[i].erase(tmp[i].begin());  // Supprime le '#'
		else
		{
			senderror(403, GetClient(fd)->GetNickName(), tmp[i], GetClient(fd)->GetFd(), " :No such channel\r\n");
			tmp.erase(tmp.begin() + i--);
		}
	}
	return 1;
}

/**
 * @brief Commande PART - quitte un ou plusieurs canaux
 * @param cmd Commande PART brute
 * @param fd File descriptor du client
 * 
 * Format : PART #canal1,#canal2 [:raison]
 * Le client est retiré des canaux spécifiés et les autres membres sont notifiés.
 */
void Server::PART(std::string cmd, int fd)
{
	std::vector<std::string> tmp;
	std::string reason;
	// Parsing de la commande
	if (!SplitCmdPart(cmd, tmp, reason, fd))
	{
		senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n");
		return;
	}
	// Traitement de chaque canal
	for (size_t i = 0; i < tmp.size(); i++)
	{
		bool flag = false;
		// Recherche du canal dans la liste des canaux du serveur
		for (size_t j = 0; j < this->channels.size(); j++)
		{
			if (this->channels[j].GetName() == tmp[i])
			{
				flag = true;
				// Vérification que le client est sur le canal (ERR_NOTONCHANNEL 442)
				if (!channels[j].get_client(fd) && !channels[j].get_admin(fd))
				{
					senderror(442, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :You're not on that channel\r\n");
					continue;
				}
					// Construction du message PART
					std::stringstream ss;
					ss << ":" << GetClient(fd)->GetNickName() << "!~" << GetClient(fd)->GetUserName() << "@" << "localhost" << " PART #" << tmp[i];
					if (!reason.empty())
						ss << " :" << reason << "\r\n";
					else ss << "\r\n";
					// Notification à tous les membres (y compris celui qui part)
					channels[j].sendTo_all(ss.str());
					// Suppression du client du canal
					if (channels[j].get_admin(channels[j].GetClientInChannel(GetClient(fd)->GetNickName())->GetFd()))
						channels[j].remove_admin(channels[j].GetClientInChannel(GetClient(fd)->GetNickName())->GetFd());
					else
						channels[j].remove_client(channels[j].GetClientInChannel(GetClient(fd)->GetNickName())->GetFd());
					// Suppression du canal s'il est vide
					if (channels[j].GetClientsNumber() == 0)
						channels.erase(channels.begin() + j);
			}
		}
		// Canal inexistant (ERR_NOSUCHCHANNEL 403)
		if (!flag)
			senderror(403, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :No such channel\r\n");
	}
}