#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                            INVITE COMMAND                                  */
/* ************************************************************************** */

/**
 * @brief Invite un utilisateur à rejoindre un canal
 * @param cmd Commande INVITE reçue (format: INVITE <nickname> <#channel>)
 * @param fd File descriptor du client émetteur
 * 
 * Codes d'erreur gérés :
 *   461 - ERR_NEEDMOREPARAMS  : paramètres insuffisants
 *   403 - ERR_NOSUCHCHANNEL   : canal inexistant
 *   442 - ERR_NOTONCHANNEL    : émetteur pas sur le canal
 *   443 - ERR_USERONCHANNEL   : cible déjà sur le canal
 *   401 - ERR_NOSUCHNICK      : nickname cible introuvable
 *   482 - ERR_CHANOPRIVSNEEDED: privilèges opérateur requis
 *   471 - ERR_CHANNELISFULL   : canal plein
 */
void Server::Invite(std::string &cmd, int &fd)
{
	std::vector<std::string> scmd = split_cmd(cmd);
	// Vérification du nombre de paramètres (INVITE <nick> <channel>)
	if(scmd.size() < 3)
	{
		senderror(461, GetClient(fd)->GetNickName(), fd, " :Not enough parameters\r\n");
		return;
	}
	// Extraction du nom de canal (sans le '#')
	std::string channelname = scmd[2].substr(1);
	// Vérification de l'existence du canal
	if(scmd[2][0] != '#' || !GetChannel(channelname))
	{
		senderror(403, channelname, fd, " :No such channel\r\n");
		return;
	}
	// Vérification que l'émetteur est membre du canal
	if (!(GetChannel(channelname)->get_client(fd)) && !(GetChannel(channelname)->get_admin(fd)))
	{
		senderror(442, channelname, fd, " :You're not on that channel\r\n");
		return;
	}
	// Vérification que la cible n'est pas déjà sur le canal
	if (GetChannel(channelname)->GetClientInChannel(scmd[1]))
	{
		senderror(443, GetClient(fd)->GetNickName(), channelname, fd, " :is already on channel\r\n");
		return;
	}
	// Recherche du client cible par nickname
	Client *clt = GetClientNick(scmd[1]);
	if (!clt)
	{
		senderror(401, scmd[1], fd, " :No such nick\r\n");
		return;
	}
	// Canal invite-only : seuls les opérateurs peuvent inviter
	if (GetChannel(channelname)->GetInvitOnly() && !GetChannel(channelname)->get_admin(fd))
	{
		senderror(482,GetChannel(channelname)->get_client(fd)->GetNickName(),scmd[1],fd," :You're not channel operator\r\n");
		return;
	}
	// Vérification de la limite de membres du canal
	if (GetChannel(channelname)->GetLimit() && GetChannel(channelname)->GetClientsNumber() >= GetChannel(channelname)->GetLimit())
	{
		senderror(473,GetChannel(channelname)->get_client(fd)->GetNickName(),channelname,fd," :Cannot invit to channel (+i)\r\n");
		return;
	}
	// Invitation réussie : ajout à la liste des invitations du client
	clt->AddChannelInvite(channelname);
	// Confirmation à l'émetteur (RPL_INVITING 341)
	std::string rep1 = ": 341 "+ GetClient(fd)->GetNickName()+" "+ clt->GetNickName()+" "+ scmd[2]+"\r\n";
	_sendResponse(rep1, fd);
	// Notification au client invité
	std::string rep2 = ":"+ GetClient(fd)->getHostname() + "@localhost INVITE " + clt->GetNickName() + " " + scmd[2]+"\r\n";
	_sendResponse(rep2, clt->GetFd());
	
}