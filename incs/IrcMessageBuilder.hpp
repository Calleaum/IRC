#pragma once

#include "IrcNumericReplies.hpp"
#include <string>
#include <sstream>

class IrcMessageBuilder
{
public:
    /* Erreurs et réponses courantes */
    
    static std::string buildNeedMoreParamsError(const std::string& serverName, const std::string& command);
    static std::string buildErrorMessage(const std::string& serverName, const std::string& errorCode, const std::string& details);
    static std::string truncateAndAppend(const std::string& message);

    /* Messages de bienvenue et informations serveur */
    static std::string buildWelcomeMessage(const std::string& serverName, const std::string& nickname, const std::string& realName, const std::string& host);
    static std::string buildYourHostMessage(const std::string& serverName, const std::string& nick, const std::string& serverVersion);
    static std::string buildServerCreatedMessage(const std::string& serverName, const std::string& nick, const std::string& creationDate);
    static std::string buildMyInfoMessage(const std::string& serverName, const std::string& nick, const std::string& version, const std::string& userModes, const std::string& channelModes);

    /* Messages MOTD */
    static std::string buildMotdStartMessage(const std::string& serverName, const std::string& nick);
    static std::string buildMotdMessage(const std::string& serverName, const std::string& nick, const std::string& message);
    static std::string buildMotdEndMessage(const std::string& serverName, const std::string& nick);

    /* Messages canal */
    static std::string buildNoSuchChannelError(const std::string& serverName, const std::string& channelName);
    static std::string buildChannelOperatorNeededError(const std::string& serverName, const std::string& nickname, const std::string& channelName);
    static std::string buildUserNotInChannelError(const std::string& serverName, const std::string& nickname, const std::string& channelName);
    static std::string buildNotOnChannelError(const std::string& serverName, const std::string& channelName);
    static std::string buildInviteOnlyChannelError(const std::string& serverName, const std::string& channelName);
    static std::string buildBadChannelKeyError(const std::string& serverName, const std::string& channelName);
    static std::string buildChannelIsFullError(const std::string& serverName, const std::string& channelName);
    static std::string buildBadChannelMaskError(const std::string& serverName, const std::string& channelName);
    
    static std::string buildJoinMessage(const std::string& nickname, const std::string& realname, const std::string& serverIp, const std::string& channelName);
    static std::string buildPartMessage(const std::string& nickname, const std::string& username, const std::string& serverIp, const std::string& channelName);
    static std::string buildKickMessage(const std::string& nickname, const std::string& channelName, const std::string& targetNick, const std::string& comment);

    /* Messages utilisateur */
    static std::string buildNoSuchNickError(const std::string& serverName, const std::string& targetNick);
    static std::string buildUserOnChannelError(const std::string& serverName, const std::string& targetNick, const std::string& channelName);
    static std::string buildErroneousNicknameError(const std::string& serverName, const std::string& newNickname);
    static std::string buildNicknameInUseError(const std::string& serverName, const std::string& newNickname);
    static std::string buildNickChangeMessage(const std::string& currentNickname, const std::string& newNickname);
    static std::string buildErroneousUsernameError(const std::string& serverName, const std::string& username);

    /* Mode / topic */
    static std::string buildUnknownModeError(const std::string& serverName, const std::string& nickname, char modeChar);
    static std::string buildModeChangeMessage(const std::string& nickname, const std::string& channelName, const std::string& modeString);
    static std::string buildTopicMessage(const std::string& nickname, const std::string& channelName, const std::string& topic);
    static std::string buildTopicReply(const std::string& serverName, const std::string& nickname, const std::string& channelName, const std::string& topic);
    static std::string buildNoTopicReply(const std::string& serverName, const std::string& nickname, const std::string& channelName);

    /* Invitations */
    static std::string buildInviteMessage(const std::string& nickname, const std::string& targetNick, const std::string& channelName);
    static std::string buildInvitingReply(const std::string& serverName, const std::string& nickname, const std::string& targetNick, const std::string& channelName);

    /* Liste des noms */
    static std::string buildNamesReply(const std::string& serverName, const std::string& nick, const std::string& channelName, const std::string& nickList);
    static std::string buildEndOfNamesMessage(const std::string& serverName, const std::string& nick, const std::string& channelName);

    /* CAP */
    static std::string buildCapabilityListMessage(const std::string& serverName, const std::string& nick, const std::string& capabilities);
    static std::string buildInvalidCapSubcommandError(const std::string& serverName, const std::string& nick, const std::string& subCommand);

    /* Autres erreurs serveur / commandes */
    static std::string buildNotRegisteredError(const std::string& serverName);
    static std::string buildUnknownCommandError(const std::string& serverName, const std::string& command);
    static std::string buildAlreadyRegisteredError(const std::string& serverName);
    static std::string buildPasswordMismatchError(const std::string& serverName);
    static std::string buildNoNicknameGivenError(const std::string& serverName);
    static std::string buildCannotSendToChannelError(const std::string& serverName, const std::string& nickname, const std::string& target);

    static std::string buildChannelModeIsResponse(const std::string& serverName, const std::string& nickname, const std::string& channelName, const std::string& modes, const std::string& modeParams);
};
