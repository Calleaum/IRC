#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                             TOPIC COMMAND                                  */
/* ************************************************************************** */

/**
 * Codes d'erreur gérés :
 *   461 - ERR_NEEDMOREPARAMS   : paramètres insuffisants
 *   403 - ERR_NOSUCHCHANNEL    : canal inexistant
 *   442 - ERR_NOTONCHANNEL     : client pas sur le canal
 *   482 - ERR_CHANOPRIVSNEEDED : privilèges opérateur requis
 *   331 - RPL_NOTOPIC          : pas de topic défini
 *   332 - RPL_TOPIC            : topic actuel
 *   333 - RPL_TOPICWHOTIME     : qui a défini le topic et quand
 */

/**
 * @brief Génère un timestamp pour le topic
 * @return Timestamp actuel en secondes (format string)
 */
std::string Server::tTopic()
{
	std::time_t current = std::time(NULL);
	std::stringstream res;

	res << current;
	return res.str();
}

/**
 * @brief Extrait le topic d'une commande (après le ':')
 * @param input Commande brute
 * @return Topic extrait ou chaîne vide
 */
std::string Server::gettopic(std::string &input)
{
	size_t pos = input.find(":");
	if (pos == std::string::npos)
	{
		return "";
	}
	return input.substr(pos);
}

/**
 * @brief Trouve la position du ':' précédé d'un espace
 * @param cmd Commande à analyser
 * @return Position du ':' ou -1 si non trouvé
 */
int Server::getpos(std::string &cmd)
{
	for (int i = 0; i < (int)cmd.size(); i++)
		if (cmd[i] == ':' && (cmd[i - 1] == 32))  // 32 = espace
			return i;
	return -1;
}

/**
 * @brief Commande TOPIC - consulte ou modifie le topic d'un canal
 * @param cmd Commande TOPIC brute
 * @param fd File descriptor du client
 * 
 * Formats :
 *   TOPIC #canal          : consulte le topic actuel
 *   TOPIC #canal :nouveau : définit un nouveau topic
 *   TOPIC #canal :        : efface le topic
 */
void Server::Topic(std::string &cmd, int &fd)
{
	// Cas spécial : "TOPIC :"
	if (cmd == "TOPIC :")
	{
		senderror(461, GetClient(fd)->GetNickName(), fd, " :Not enough parameters\r\n");
		return;
	}
	std::vector<std::string> scmd = split_cmd(cmd);
	// Vérification des paramètres minimum
	if (scmd.size() == 1)
	{
		senderror(461, GetClient(fd)->GetNickName(), fd, " :Not enough parameters\r\n");
		return;
	}
	// Extraction du nom de canal (sans '#')
	std::string nmch = scmd[1].substr(1);
	// Vérification de l'existence du canal (ERR_NOSUCHCHANNEL 403)
	if (!GetChannel(nmch))
	{
		senderror(403, "#" + nmch, fd, " :No such channel\r\n");
		return;
	}
	// Vérification que le client est sur le canal (ERR_NOTONCHANNEL 442)
	if (!(GetChannel(nmch)->get_client(fd)) && !(GetChannel(nmch)->get_admin(fd)))
	{
		senderror(442, "#" + nmch, fd, " :You're not on that channel\r\n");
		return;
	}
	// Consultation du topic (TOPIC #canal sans argument)
	if (scmd.size() == 2)
	{
		// Pas de topic défini (RPL_NOTOPIC 331)
		if (GetChannel(nmch)->GetTopicName() == "")
		{
			_sendResponse(": 331 " + GetClient(fd)->GetNickName() + " " + "#" + nmch + " :No topic is set\r\n", fd);
			return;
		}
		// Topic défini (RPL_TOPIC 332 + RPL_TOPICWHOTIME 333)
		size_t pos = GetChannel(nmch)->GetTopicName().find(":");
		if (GetChannel(nmch)->GetTopicName() != "" && pos == std::string::npos)
		{
			_sendResponse(": 332 " + GetClient(fd)->GetNickName() + " " + "#" + nmch + " " + GetChannel(nmch)->GetTopicName() + "\r\n", fd);
			_sendResponse(": 333 " + GetClient(fd)->GetNickName() + " " + "#" + nmch + " " + GetClient(fd)->GetNickName() + " " + GetChannel(nmch)->GetTime() + "\r\n", fd);
			return;
		}
		else
		{
			size_t pos = GetChannel(nmch)->GetTopicName().find(" ");
			if (pos == 0)
				GetChannel(nmch)->GetTopicName().erase(0, 1);
			_sendResponse(": 332 " + GetClient(fd)->GetNickName() + " " + "#" + nmch + " " + GetChannel(nmch)->GetTopicName() + "\r\n", fd);
			_sendResponse(": 333 " + GetClient(fd)->GetNickName() + " " + "#" + nmch + " " + GetClient(fd)->GetNickName() + " " + GetChannel(nmch)->GetTime() + "\r\n", fd);
			return;
		}
	}

	// Modification du topic (TOPIC #canal :nouveau_topic)
	if (scmd.size() >= 3)
	{
		std::vector<std::string> tmp;
		int pos = getpos(cmd);
		// Parsing du nouveau topic
		if (pos == -1 || scmd[2][0] != ':')
		{
			tmp.push_back(scmd[0]);
			tmp.push_back(scmd[1]);
			tmp.push_back(scmd[2]);
		}
		else
		{
			tmp.push_back(scmd[0]);
			tmp.push_back(scmd[1]);
			tmp.push_back(cmd.substr(getpos(cmd)));
		}

		// Topic vide ":" - effacement (RPL_NOTOPIC 331)
		if (tmp[2][0] == ':' && tmp[2][1] == '\0')
		{
			senderror(331, "#" + nmch, fd, " :No topic is set\r\n");
			return;
		}

		// Mode +t actif et client n'est pas opérateur (ERR_CHANOPRIVSNEEDED 482)
		if (GetChannel(nmch)->Gettopic_restriction() && GetChannel(nmch)->get_client(fd))
		{
			senderror(482, "#" + nmch, fd, " :You're Not a channel operator\r\n");
			return;
		}
		// Mode +t actif et client est opérateur : modification autorisée
		else if (GetChannel(nmch)->Gettopic_restriction() && GetChannel(nmch)->get_admin(fd))
		{
			GetChannel(nmch)->SetTime(tTopic());
			GetChannel(nmch)->SetTopicName(tmp[2]);
			std::string rpl;
			size_t pos = tmp[2].find(":");
			if (pos == std::string::npos)
				rpl = ":" + GetClient(fd)->GetNickName() + "!" + GetClient(fd)->GetUserName() + "@localhost TOPIC #" + nmch + " :" + GetChannel(nmch)->GetTopicName() + "\r\n";
			else
				rpl = ":" + GetClient(fd)->GetNickName() + "!" + GetClient(fd)->GetUserName() + "@localhost TOPIC #" + nmch + " " + GetChannel(nmch)->GetTopicName() + "\r\n";
			// Notification à tous les membres du canal
			GetChannel(nmch)->sendTo_all(rpl);
		}
		// Mode +t inactif : tout le monde peut modifier le topic
		else
		{
			std::string rpl;
			size_t pos = tmp[2].find(":");
			if (pos == std::string::npos)
			{
				GetChannel(nmch)->SetTime(tTopic());
				GetChannel(nmch)->SetTopicName(tmp[2]);
				rpl = ":" + GetClient(fd)->GetNickName() + "!" + GetClient(fd)->GetUserName() + "@localhost TOPIC #" + nmch + " " + GetChannel(nmch)->GetTopicName() + "\r\n";
			}
			else
			{
				size_t poss = tmp[2].find(" ");
				GetChannel(nmch)->SetTopicName(tmp[2]);
				// Nettoyage du format du topic
				if (poss == std::string::npos && tmp[2][0] == ':' && tmp[2][1] != ':')
					tmp[2] = tmp[2].substr(1);
				GetChannel(nmch)->SetTopicName(tmp[2]);
				GetChannel(nmch)->SetTime(tTopic());
				rpl = ":" + GetClient(fd)->GetNickName() + "!" + GetClient(fd)->GetUserName() + "@localhost TOPIC #" + nmch + " " + GetChannel(nmch)->GetTopicName() + "\r\n";
			}
			// Notification à tous les membres du canal
			GetChannel(nmch)->sendTo_all(rpl);
		}
	}
}