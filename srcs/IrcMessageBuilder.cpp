#include "IrcMessageBuilder.hpp"

// Coupe le message à 510 chars et ajoute "\r\n"
std::string IrcMessageBuilder::truncateAndAppend(const std::string& message) {
    const size_t maxLength = 510;
    if (message.length() > maxLength)
        return message.substr(0, maxLength) + "\r\n";
    return message + "\r\n";
}

// Erreur : pas assez de paramètres
std::string IrcMessageBuilder::buildNeedMoreParamsError(const std::string& serverName, const std::string& command)
{
    std::ostringstream oss;
    oss << ":" << serverName << ERR_NEEDMOREPARAMS << " " << command << " :Not enough parameters";
    return truncateAndAppend(oss.str());
}

// Construit un message d'erreur générique
std::string IrcMessageBuilder::buildErrorMessage(const std::string& serverName, const std::string& errorCode, const std::string& details) {
    std::ostringstream oss;
    oss << ":" << serverName << " " << errorCode << " " << details;
    return truncateAndAppend(oss.str());
}

// Message de bienvenue
std::string IrcMessageBuilder::buildWelcomeMessage(const std::string& serverName, const std::string& nickname, const std::string& realName, const std::string& host) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_WELCOME << nickname 
        << " :Welcome to the Internet Relay Network " 
        << nickname << "!" << realName << "@" << host;
    return truncateAndAppend(oss.str());
}

// Erreur : canal inexistant
std::string IrcMessageBuilder::buildNoSuchChannelError(const std::string& serverName, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_NOSUCHCHANNEL << channelName << " :No such channel";
    return truncateAndAppend(oss.str());
}

// Erreur : pas opérateur du canal
std::string IrcMessageBuilder::buildChannelOperatorNeededError(const std::string& serverName, const std::string& nickname, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_CHANOPRIVSNEEDED << nickname << " " << channelName << " :You're not channel operator";
    return truncateAndAppend(oss.str());
}

// Erreur : l’utilisateur n’est pas sur le canal
std::string IrcMessageBuilder::buildUserNotInChannelError(const std::string& serverName, const std::string& nickname, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_USERNOTINCHANNEL << nickname << " " << channelName << " :They aren't on that channel";
    return truncateAndAppend(oss.str());
}

// Erreur : mode inconnu
std::string IrcMessageBuilder::buildUnknownModeError(const std::string& serverName, const std::string& nickname, char modeChar) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_UNKNOWNMODE << nickname << " " << modeChar << " :is unknown mode char to me";
    return truncateAndAppend(oss.str());
}

// Message MODE
std::string IrcMessageBuilder::buildModeChangeMessage(const std::string& nickname, const std::string& channelName, const std::string& modeString) {
    std::ostringstream oss;
    oss << ":" << nickname << " MODE " << channelName << " " << modeString;
    return truncateAndAppend(oss.str());
}

// Erreur : pas sur le canal
std::string IrcMessageBuilder::buildNotOnChannelError(const std::string& serverName, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_NOTONCHANNEL << channelName << " :You're not on that channel";
    return truncateAndAppend(oss.str());
}

// Erreur : nickname inexistant
std::string IrcMessageBuilder::buildNoSuchNickError(const std::string& serverName, const std::string& targetNick) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_NOSUCHNICK << targetNick << " :No such nick/channel";
    return truncateAndAppend(oss.str());
}

// Erreur : déjà sur le canal
std::string IrcMessageBuilder::buildUserOnChannelError(const std::string& serverName, const std::string& targetNick, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_USERONCHANNEL << targetNick << " " << channelName << " :is already on channel";
    return truncateAndAppend(oss.str());
}

// Message INVITE
std::string IrcMessageBuilder::buildInviteMessage(const std::string& nickname, const std::string& targetNick, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << nickname << " INVITE " << targetNick << " :" << channelName;
    return truncateAndAppend(oss.str());
}

// Réponse RPL_INVITING
std::string IrcMessageBuilder::buildInvitingReply(const std::string& serverName, const std::string& nickname, const std::string& targetNick, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_INVITING << nickname << " " << targetNick << " :" << channelName;
    return truncateAndAppend(oss.str());
}

// Réponse TOPIC
std::string IrcMessageBuilder::buildTopicReply(const std::string& serverName, const std::string& nickname, const std::string& channelName, const std::string& topic) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_TOPIC << nickname << " " << channelName << " :" << topic;
    return truncateAndAppend(oss.str());
}

// Réponse : pas de topic
std::string IrcMessageBuilder::buildNoTopicReply(const std::string& serverName, const std::string& nickname, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_NOTOPIC << nickname << " " << channelName << " :No topic is set";
    return truncateAndAppend(oss.str());
}

// Message TOPIC
std::string IrcMessageBuilder::buildTopicMessage(const std::string& nickname, const std::string& channelName, const std::string& topic) {
    std::ostringstream oss;
    oss << ":" << nickname << " TOPIC " << channelName << " :" << topic;
    return truncateAndAppend(oss.str());
}

// Message KICK
std::string IrcMessageBuilder::buildKickMessage(const std::string& nickname, const std::string& channelName, const std::string& targetNick, const std::string& comment) {
    std::ostringstream oss;
    oss << ":" << nickname << " KICK " << channelName << " " << targetNick << " :" << comment;
    return truncateAndAppend(oss.str());
}

// Message CAP LS
std::string IrcMessageBuilder::buildCapabilityListMessage(const std::string& serverName, const std::string& nick, const std::string& capabilities) {
    std::ostringstream oss;
    oss << ":" << serverName << " CAP " << nick << " LS :" << capabilities;
    return truncateAndAppend(oss.str());
}

// Erreur CAP invalide
std::string IrcMessageBuilder::buildInvalidCapSubcommandError(const std::string& serverName, const std::string& nick, const std::string& subCommand) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_INVALIDCAPCMD << nick << " " << subCommand << " :Invalid CAP subcommand";
    return truncateAndAppend(oss.str());
}

// Erreur : pas encore enregistré
std::string IrcMessageBuilder::buildNotRegisteredError(const std::string& serverName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_NOTREGISTERED << ":You have not registered";
    return truncateAndAppend(oss.str());
}

// Erreur : commande inconnue
std::string IrcMessageBuilder::buildUnknownCommandError(const std::string& serverName, const std::string& command) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_UNKNOWNCOMMAND << command << " :Unknown command";
    return truncateAndAppend(oss.str());
}

// Erreur : déjà enregistré
std::string IrcMessageBuilder::buildAlreadyRegisteredError(const std::string& serverName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_ALREADYREGISTRED << ":You may not reregister";
    return truncateAndAppend(oss.str());
}

// Erreur : mauvais mot de passe
std::string IrcMessageBuilder::buildPasswordMismatchError(const std::string& serverName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_PASSWDMISMATCH << ":Password incorrect";
    return truncateAndAppend(oss.str());
}

// Erreur : pas de nick donné
std::string IrcMessageBuilder::buildNoNicknameGivenError(const std::string& serverName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_NONICKNAMEGIVEN << ":No nickname given";
    return truncateAndAppend(oss.str());
}

// Erreur : nick invalide
std::string IrcMessageBuilder::buildErroneousNicknameError(const std::string& serverName, const std::string& newNickname) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_ERRONEUSNICKNAME << newNickname << " :Erroneous nickname";
    return truncateAndAppend(oss.str());
}

// Erreur : nick déjà utilisé
std::string IrcMessageBuilder::buildNicknameInUseError(const std::string& serverName, const std::string& newNickname) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_NICKNAMEINUSE << newNickname << " :Nickname is already in use";
    return truncateAndAppend(oss.str());
}

// Message NICK
std::string IrcMessageBuilder::buildNickChangeMessage(const std::string& currentNickname, const std::string& newNickname) {
    std::ostringstream oss;
    oss << ":" << currentNickname << " NICK :" << newNickname << ".\033[0m";
    return truncateAndAppend(oss.str());
}

// Erreur : username invalide
std::string IrcMessageBuilder::buildErroneousUsernameError(const std::string& serverName, const std::string& username) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_ERRONEUSUSERNAME << username << " :Erroneous username";
    return truncateAndAppend(oss.str());
}

// Message : info du serveur
std::string IrcMessageBuilder::buildYourHostMessage(const std::string& serverName, const std::string& nick, const std::string& serverVersion) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_YOURHOST << nick 
        << " :Your host is " << serverName << ", running version " << serverVersion;
    return truncateAndAppend(oss.str());
}

// Message : date de création du serveur
std::string IrcMessageBuilder::buildServerCreatedMessage(const std::string& serverName, const std::string& nick, const std::string& creationDate) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_CREATED << nick 
        << " :This server was created " << creationDate;
    return truncateAndAppend(oss.str());
}

// Message RPL_MYINFO
std::string IrcMessageBuilder::buildMyInfoMessage(const std::string& serverName, const std::string& nick, const std::string& version, const std::string& userModes, const std::string& channelModes) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_MYINFO << nick << " " << serverName << " " << version 
        << " " << userModes << " " << channelModes;
    return truncateAndAppend(oss.str());
}

// Début du MOTD
std::string IrcMessageBuilder::buildMotdStartMessage(const std::string& serverName, const std::string& nick) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_MOTDSTART << nick << " :- " << serverName << " Message of the Day -";
    return truncateAndAppend(oss.str());
}

// Ligne du MOTD
std::string IrcMessageBuilder::buildMotdMessage(const std::string& serverName, const std::string& nick, const std::string& message) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_MOTD << nick << " :- " << message;
    return truncateAndAppend(oss.str());
}

// Fin du MOTD
std::string IrcMessageBuilder::buildMotdEndMessage(const std::string& serverName, const std::string& nick) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_ENDOFMOTD << nick << " :End of /MOTD command.";
    return truncateAndAppend(oss.str());
}

// Liste des noms
std::string IrcMessageBuilder::buildNamesReply(const std::string& serverName, const std::string& nick, const std::string& channelName, const std::string& nickList) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_NAMREPLY << nick << " = " << channelName << " :" << nickList;
    return truncateAndAppend(oss.str());
}

// Fin de la liste des noms
std::string IrcMessageBuilder::buildEndOfNamesMessage(const std::string& serverName, const std::string& nick, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << RPL_ENDOFNAMES << nick << " " << channelName << " :End of /NAMES list.";
    return truncateAndAppend(oss.str());
}

// Erreur : mauvais nom de canal
std::string IrcMessageBuilder::buildBadChannelMaskError(const std::string& serverName, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_BADCHANMASK << channelName << " :Bad Channel Mask";
    return truncateAndAppend(oss.str());
}

// Erreur : canal en mode +i
std::string IrcMessageBuilder::buildInviteOnlyChannelError(const std::string& serverName, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_INVITEONLYCHAN << channelName << " :Cannot join channel (+i)";
    return truncateAndAppend(oss.str());
}

// Erreur : canal protégé par clé
std::string IrcMessageBuilder::buildBadChannelKeyError(const std::string& serverName, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_BADCHANNELKEY << channelName << " :Cannot join channel (+k)";
    return truncateAndAppend(oss.str());
}

// Erreur : canal plein
std::string IrcMessageBuilder::buildChannelIsFullError(const std::string& serverName, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_CHANNELISFULL << channelName << " :Cannot join channel (+l)";
    return truncateAndAppend(oss.str());
}

// Message JOIN
std::string IrcMessageBuilder::buildJoinMessage(const std::string& nickname, const std::string& realname, const std::string& serverIp, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << nickname << "!" << realname << "@" << serverIp << " JOIN :" << channelName;
    return truncateAndAppend(oss.str());
}

// Message PART
std::string IrcMessageBuilder::buildPartMessage(const std::string& nickname, const std::string& username, const std::string& serverIp, const std::string& channelName) {
    std::ostringstream oss;
    oss << ":" << nickname << "!" << username << "@" << serverIp << " PART " << channelName;
    return truncateAndAppend(oss.str());
}

// Erreur : impossible d’envoyer au canal
std::string IrcMessageBuilder::buildCannotSendToChannelError(const std::string& serverName, const std::string& nickname, const std::string& target) {
    std::ostringstream oss;
    oss << ":" << serverName << ERR_CANNOTSENDTOCHAN << nickname << " " << target << " :Cannot send to channel";
    return truncateAndAppend(oss.str());
}

// Réponse MODE du canal
std::string IrcMessageBuilder::buildChannelModeIsResponse(
    const std::string& serverName,
    const std::string& nickname,
    const std::string& channelName,
    const std::string& modes,
    const std::string& modeParams
)
{
    std::ostringstream oss;
    oss << ":" << serverName << RPL_CHANNELMODEIS << nickname << " " << channelName << " " << modes;
    if (!modeParams.empty())
        oss << " " << modeParams;
    return truncateAndAppend(oss.str());
}
