#pragma once
#include <string>

/* -------------------------------------------------------------------------- */
/*                                  ERROR CODES                                */
/* -------------------------------------------------------------------------- */

/* RPL_WELCOME : "001 :Welcome to the Internet Relay Network nickname!realName@host\r\n" */
const std::string RPL_WELCOME = "001";
const std::string RPL_WELCOME_MSG = " :Welcome to the Internet Relay Network nickname!realName@host\r\n";

const std::string RPL_YOURHOST = "002";
const std::string RPL_YOURHOST_MSG = " :Your host is serverName, running version serverVersion\r\n";

const std::string RPL_CREATED = "003";
const std::string RPL_CREATED_MSG = " :This server was created at some point in the past\r\n";

const std::string RPL_MYINFO = "004";
const std::string RPL_MYINFO_MSG = " serverName 1.0 o o\r\n";

/* 4xx ERRORS */
const std::string ERR_NOSUCHNICK = "401";
const std::string ERR_NOSUCHNICK_MSG = " <nickname> :No such nick/channel\r\n";

const std::string ERR_NOSUCHSERVER = "402";
const std::string ERR_NOSUCHSERVER_MSG = " <server> :No such server\r\n";

const std::string ERR_NOSUCHCHANNEL = "403";
const std::string ERR_NOSUCHCHANNEL_MSG = " <channel> :No such channel\r\n";

const std::string ERR_CANNOTSENDTOCHAN = "404";
const std::string ERR_CANNOTSENDTOCHAN_MSG = " <channel> :Cannot send to channel\r\n";

/* ... Tous les autres codes d'erreurs 4xx comme ERR_TOOMANYCHANNELS, ERR_NEEDMOREPARAMS ... */

/* 5xx ERRORS */
const std::string ERR_NOPRIVILEGES = "481";
const std::string ERR_NOPRIVILEGES_MSG = " :Permission Denied- You're not an IRC operator\r\n";

const std::string ERR_CHANOPRIVSNEEDED = "482";
const std::string ERR_CHANOPRIVSNEEDED_MSG = " <channel> :You're not channel operator\r\n";

const std::string ERR_UMODEUNKNOWNFLAG = "501";
const std::string ERR_UMODEUNKNOWNFLAG_MSG = " :Unknown MODE flag\r\n";

const std::string ERR_USERSDONTMATCH = "502";
const std::string ERR_USERSDONTMATCH_MSG = " :Can't change mode for other users\r\n";
const std::string ERR_NEEDMOREPARAMS = "461";
const std::string ERR_ALREADYREGISTRED = "462";
const std::string ERR_PASSWDMISMATCH = "464";
const std::string ERR_NONICKNAMEGIVEN = "431";
const std::string ERR_ERRONEUSNICKNAME = "432";
const std::string ERR_NICKNAMEINUSE = "433";
const std::string ERR_USERNOTINCHANNEL = "441";
const std::string ERR_NOTONCHANNEL = "442";
const std::string ERR_USERONCHANNEL = "443";
const std::string ERR_INVITEONLYCHAN = "473";
const std::string ERR_BADCHANNELKEY = "475";
const std::string ERR_CHANNELISFULL = "471";
const std::string ERR_BADCHANMASK = "476";
const std::string ERR_UNKNOWNMODE = "472";
const std::string ERR_UNKNOWNCOMMAND = "421";
const std::string ERR_NOTREGISTERED = "451";
const std::string ERR_INVALIDCAPCMD = "410";
const std::string ERR_ERRONEUSUSERNAME = "468";

/* -------------------------------------------------------------------------- */
/*                                 REPLY CODES                                 */
/* -------------------------------------------------------------------------- */

const std::string RPL_NONE = "300";
const std::string RPL_NONE_MSG = " :No text\r\n";

const std::string RPL_USERHOST = "302";
const std::string RPL_USERHOST_MSG = " :[<reply>{<space><reply>}]\r\n";

const std::string RPL_ISON = "303";
const std::string RPL_ISON_MSG = " :[<nick> {<space><nick>}]\r\n";

const std::string RPL_AWAY = "301";
const std::string RPL_AWAY_MSG = " <nick> :<away message>\r\n";

const std::string RPL_NOTOPIC = "331";
const std::string RPL_TOPIC = "332";
const std::string RPL_INVITING = "341";
const std::string RPL_NAMREPLY = "353";
const std::string RPL_ENDOFNAMES = "366";
const std::string RPL_CHANNELMODEIS = "324";
const std::string RPL_MOTDSTART = "375";
const std::string RPL_MOTD = "372";
const std::string RPL_ENDOFMOTD = "376";

/* ... Tous les autres codes RPL comme RPL_WHOISUSER, RPL_TOPIC, RPL_ENDOFMOTD ... */

const std::string RPL_NOUSERS = "395";
const std::string RPL_NOUSERS_MSG = " :Nobody logged in\r\n";
