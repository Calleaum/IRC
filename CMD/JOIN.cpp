#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                              JOIN COMMAND                                  */
/* ************************************************************************** */

/**
 * @brief Parse la commande JOIN et extrait les paires canal/mot de passe
 * @param token Vecteur de sortie contenant les paires (canal, password)
 * @param cmd Commande JOIN brute
 * @param fd File descriptor du client
 * @return 1 si parsing réussi, 0 si paramètres insuffisants
 * 
 * Format supporté : JOIN #chan1,#chan2 pass1,pass2
 */
int Server::SplitJoin(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd)
{
	std::vector<std::string> tmp;
	std::string ChStr, PassStr, buff;
	std::istringstream iss(cmd);
	// Extraction des mots de la commande
	while(iss >> cmd)
		tmp.push_back(cmd);
	// Vérification du nombre minimum de paramètres
	if (tmp.size() < 2)
	{
		token.clear();
		return 0;
	}
	// Suppression de "JOIN" et extraction canal/password
	tmp.erase(tmp.begin());
	ChStr = tmp[0]; tmp.erase(tmp.begin());
	if (!tmp.empty())
	{
		PassStr = tmp[0];
		tmp.clear();
	}
	// Parsing des noms de canaux séparés par ','
	for (size_t i = 0; i < ChStr.size(); i++)
	{
		if (ChStr[i] == ',')
		{
			token.push_back(std::make_pair(buff, ""));
			buff.clear();
		}
		else buff += ChStr[i];
	}
	token.push_back(std::make_pair(buff, ""));
	// Association des mots de passe aux canaux
	if (!PassStr.empty())
	{
		size_t j = 0; buff.clear();
		for (size_t i = 0; i < PassStr.size(); i++)
		{
			if (PassStr[i] == ',')
			{
				token[j].second = buff; j++;
				buff.clear();
			}
			else buff += PassStr[i];
		}
		token[j].second = buff;
	}
	// Nettoyage des entrées vides
	for (size_t i = 0; i < token.size(); i++)
	{
		if (token[i].first.empty())
			token.erase(token.begin() + i--);
	}
	// Validation du format des noms de canaux (doit commencer par '#')
	for (size_t i = 0; i < token.size(); i++)
	{
		if (*(token[i].first.begin()) != '#')
			{
				senderror(403, GetClient(fd)->GetNickName(), token[i].first, GetClient(fd)->GetFd(), " :No such channel\r\n");
				token.erase(token.begin() + i--);
			}
		else
			token[i].first.erase(token[i].first.begin());  // Suppression du '#'
	}
	return 1;
}

/**
 * @brief Compte le nombre de canaux où un client est présent
 * @param nickname Nickname du client à rechercher
 * @return Nombre de canaux où le client est membre
 */
int Server::SearchForClients(std::string nickname)
{
	int count = 0;
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i].GetClientInChannel(nickname))
			count++;
	}
	return count;
}

/**
 * @brief Vérifie si un client a été invité sur un canal
 * @param cli Pointeur vers le client
 * @param ChName Nom du canal
 * @param flag Si 1, supprime l'invitation après vérification
 * @return true si invité, false sinon
 */
bool IsInvited(Client *cli, std::string ChName, int flag){
	if(cli->GetInviteChannel(ChName))
	{
		if (flag == 1)
			cli->RmChannelInvite(ChName);  // Consomme l'invitation
		return true;
	}
	return false;
}

/**
 * @brief Gère la jonction à un canal existant
 * @param token Vecteur des paires canal/password
 * @param i Index du canal dans token
 * @param j Index du canal dans la liste des canaux serveur
 * @param fd File descriptor du client
 * 
 * Vérifie : présence existante, limite de canaux, mot de passe, invite-only, limite membres
 */
void Server::ExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd)
{
	// Client déjà membre du canal
	if (this->channels[j].GetClientInChannel(GetClient(fd)->GetNickName()))
		return;
	// Limite de 10 canaux par client (ERR_TOOMANYCHANNELS 405)
	if (SearchForClients(GetClient(fd)->GetNickName()) >= 10)
	{
		senderror(405, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :You have joined too many channels\r\n");
		return;
	}
	// Vérification du mot de passe (ERR_BADCHANNELKEY 475)
	if (!this->channels[j].GetPassword().empty() && this->channels[j].GetPassword() != token[i].second){
		if (!IsInvited(GetClient(fd), token[i].first, 0))
		{
			senderror(475, GetClient(fd)->GetNickName(), "#" + token[i].first, GetClient(fd)->GetFd(), " :Cannot join channel (+k) - bad key\r\n");
			return;
		}
	}
	// Canal invite-only (ERR_INVITEONLYCHAN 473)
	if (this->channels[j].GetInvitOnly()){
		if (!IsInvited(GetClient(fd), token[i].first, 1))
		{
			senderror(473, GetClient(fd)->GetNickName(), "#" + token[i].first, GetClient(fd)->GetFd(), " :Cannot join channel (+i)\r\n");
			return;
		}
	}
	// Limite de membres atteinte (ERR_CHANNELISFULL 471)
	if (this->channels[j].GetLimit() && this->channels[j].GetClientsNumber() >= this->channels[j].GetLimit())
	{
		senderror(471, GetClient(fd)->GetNickName(), "#" + token[i].first, GetClient(fd)->GetFd(), " :Cannot join channel (+l)\r\n");
		return;
	}
	// Ajout du client au canal
	Client *cli = GetClient(fd);
	this->channels[j].add_client(*cli);
	// Envoi des messages de bienvenue (avec ou sans topic)
	if(channels[j].GetTopicName().empty())
		_sendResponse(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),token[i].first) + \
			RPL_NAMREPLY(GetClient(fd)->GetNickName(),channels[j].GetName(),channels[j].clientChannel_list()) + \
			RPL_ENDOFNAMES(GetClient(fd)->GetNickName(),channels[j].GetName()),fd);
	else
		_sendResponse(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),token[i].first) + \
			RPL_TOPICIS(GetClient(fd)->GetNickName(),channels[j].GetName(),channels[j].GetTopicName()) + \
			RPL_NAMREPLY(GetClient(fd)->GetNickName(),channels[j].GetName(),channels[j].clientChannel_list()) + \
			RPL_ENDOFNAMES(GetClient(fd)->GetNickName(),channels[j].GetName()),fd);
	// Notification aux autres membres du canal
    channels[j].sendTo_all(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),token[i].first), fd);
}


/**
 * @brief Crée un nouveau canal et y ajoute le client comme admin
 * @param token Vecteur des paires canal/password
 * @param i Index du canal dans token
 * @param fd File descriptor du client créateur
 * 
 * Le créateur devient automatiquement opérateur du canal.
 */
void Server::NotExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int fd)
{
	// Vérification de la limite de canaux par client
	if (SearchForClients(GetClient(fd)->GetNickName()) >= 10)
	{
		senderror(405, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :You have joined too many channels\r\n");
		return;
	}
	// Création du nouveau canal
	Channel newChannel;
	newChannel.SetName(token[i].first);
	newChannel.add_admin(*GetClient(fd));  // Créateur = opérateur
	newChannel.set_createiontime();
	this->channels.push_back(newChannel);
	// Notification de création au client
    _sendResponse(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),newChannel.GetName()) + \
        RPL_NAMREPLY(GetClient(fd)->GetNickName(),newChannel.GetName(),newChannel.clientChannel_list()) + \
        RPL_ENDOFNAMES(GetClient(fd)->GetNickName(),newChannel.GetName()),fd);
}

/**
 * @brief Commande JOIN - rejoindre un ou plusieurs canaux
 * @param cmd Commande JOIN brute
 * @param fd File descriptor du client
 * 
 * Format : JOIN #canal1,#canal2 [password1,password2]
 * Crée le canal s'il n'existe pas, sinon rejoint l'existant.
 */
void Server::JOIN(std::string cmd, int fd)
{
	std::vector<std::pair<std::string, std::string> > token;
	// Parsing de la commande
	if (!SplitJoin(token, cmd, fd))
	{
		senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n");
		return;
	}
	// Limite de 10 canaux par commande (ERR_TOOMANYTARGETS 407)
	if (token.size() > 10)
	{
		senderror(407, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Too many channels\r\n");
		return;
	}
	// Traitement de chaque canal demandé
	for (size_t i = 0; i < token.size(); i++){
		bool flag = false;
		// Recherche si le canal existe déjà
		for (size_t j = 0; j < this->channels.size(); j++){
			if (this->channels[j].GetName() == token[i].first){
				ExistCh(token, i, j, fd);  // Canal existant
				flag = true; break;
			}
		}
		if (!flag)
			NotExistCh(token, i, fd);  // Création du canal
	}
}