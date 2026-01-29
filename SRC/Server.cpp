#include "../INC/Server.hpp"

/* ************************************************************************** */
/*                         CONSTRUCTORS / DESTRUCTOR                          */
/* ************************************************************************** */

bool Server::isBotfull = false;

Server::Server()
{this->server_fdsocket = -1;}

Server::~Server()
{}

Server::Server(Server const &src)
{*this = src;}

Server &Server::operator=(Server const &src)
{
	if (this != &src)
	{
		/*
		struct sockaddr_in add;
		struct sockaddr_in cliadd;
		struct pollfd new_cli;
		*/
		this->port = src.port;
		this->server_fdsocket = src.server_fdsocket;
		this->password = src.password;
		this->clients = src.clients;
		this->channels = src.channels;
		this->fds = src.fds;
		this->isBotfull = src.isBotfull;
	}
	return *this;
}

//---------------//Getters
int Server::GetPort()
{return this->port;}

int Server::GetFd()
{return this->server_fdsocket;}

/**
 * @brief Recherche un client par son file descriptor
 * @param fd File descriptor du socket client
 * @return Pointeur vers le Client ou NULL si non trouvé
 */
Client *Server::GetClient(int fd)
{
	// Parcours de la liste des clients connectés
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		// Correspondance trouvée par file descriptor
		if (this->clients[i].GetFd() == fd)
			return &this->clients[i];
	}
	return NULL;
}

/**
 * @brief Comparaison de chaînes insensible à la casse
 */
static bool iequals(const std::string& a, const std::string& b)
{
	// Tailles différentes = chaînes différentes
	if (a.size() != b.size()) return false;
	// Comparaison caractère par caractère en minuscules
	for (size_t i = 0; i < a.size(); i++)
	{
		if (std::tolower(a[i]) != std::tolower(b[i]))
			return false;
	}
	return true;
}

/**
 * @brief Recherche un client par son nickname (insensible à la casse)
 * @param nickname Nickname à rechercher
 * @return Pointeur vers le Client ou NULL si non trouvé
 */
Client *Server::GetClientNick(std::string nickname)
{
	for (size_t i = 0; i < this->clients.size(); i++){
		if (iequals(this->clients[i].GetNickName(), nickname))
			return &this->clients[i];
	}
	return NULL;
}

/**
 * @brief Recherche un canal par son nom
 * @param name Nom du canal (sensible à la casse)
 * @return Pointeur vers le Channel ou NULL si non trouvé
 */
Channel *Server::GetChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i].GetName() == name)
			return &channels[i];
	}
	return NULL;
}

//---------------//Getters
//---------------//Setters
void Server::SetFd(int fd)
{this->server_fdsocket = fd;}

void Server::SetPort(int port)
{this->port = port;}

void Server::SetPassword(std::string password)
{this->password = password;}

std::string Server::GetPassword()
{return this->password;}

void Server::AddClient(Client newClient)
{this->clients.push_back(newClient);}

void Server::AddChannel(Channel newChannel)
{this->channels.push_back(newChannel);}

void Server::AddFds(pollfd newFd)
{this->fds.push_back(newFd);}

//---------------//Setters
//---------------//Remove Methods
/**
 * @brief Supprime un client de la liste par son file descriptor
 * @param fd File descriptor du client à supprimer
 */
void Server::RemoveClient(int fd)
{
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if (this->clients[i].GetFd() == fd)
		{
			this->clients.erase(this->clients.begin() + i);
			return;
		}
	}
}

void Server::RemoveChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i].GetName() == name)
		{
			this->channels.erase(this->channels.begin() + i);
			return;
		}
	}
}

/**
 * @brief Supprime une entrée pollfd de la liste de surveillance
 * @param fd File descriptor à retirer du polling
 */
void Server::RemoveFds(int fd)
{
	for (size_t i = 0; i < this->fds.size(); i++)
	{
		if (this->fds[i].fd == fd)
		{
			this->fds.erase(this->fds.begin() + i);
			return;
		}
	}
}

/**
 * @brief Retire un client de tous les canaux lors de sa déconnexion
 * @param fd File descriptor du client qui quitte
 * 
 * Notifie les membres restants et supprime les canaux devenus vides.
 */
void	Server::RmChannels(int fd){
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		int flag = 0;
		// Vérifie si le client est un membre normal du canal
		if (channels[i].get_client(fd))
		{
			channels[i].remove_client(fd);
			flag = 1;
		}
		// Vérifie si le client est un admin du canal
		else if (channels[i].get_admin(fd))
		{
			channels[i].remove_admin(fd);
			flag = 1;
		}
		// Suppression du canal s'il est vide
		if (channels[i].GetClientsNumber() == 0)
		{
			channels.erase(channels.begin() + i);
			i--;  // Ajustement de l'index après suppression
			continue;
		}
		// Notification de départ aux membres restants
		if (flag)
		{
			std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT Quit\r\n";
			channels[i].sendTo_all(rpl);
		}
	}
}

//---------------//Remove Methods
//---------------//Send Methods
void Server::senderror(int code, std::string clientname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << " " << channelname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

/**
 * @brief Envoie une réponse IRC formatée au client
 * @param response Message à envoyer (doit inclure \r\n)
 * @param fd File descriptor du destinataire
 */
void Server::_sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}

//---------------//Send Methods
//---------------//Close and Signal Methods
bool Server::Signal = false;

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true;
}

void	Server::close_fds(){
	// Fermeture de tous les sockets clients
	for(size_t i = 0; i < clients.size(); i++)
	{
		std::cout << RED << "Client <" << clients[i].GetFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].GetFd());
	}
	// Fermeture du socket serveur
	if (server_fdsocket != -1)
	{	
		std::cout << RED << "Server <" << server_fdsocket << "> Disconnected" << WHI << std::endl;
		close(server_fdsocket);
	}
}

//---------------//Close and Signal Methods
//---------------//Server Methods
void Server::init(int port, std::string pass)
{
	this->password = pass;
	this->port = port;
	// Configuration et création du socket serveur
	this->set_sever_socket();

	std::cout << GRE << "Server <" << server_fdsocket << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	// Boucle principale : attente d'événements sur les sockets
	while (Server::Signal == false)
	{
		// poll() bloque jusqu'à ce qu'un événement survienne
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));
		// Traitement de chaque socket ayant des données disponibles
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				// Nouvelle connexion sur le socket serveur
				if (fds[i].fd == server_fdsocket)
					this->accept_new_client();
				// Données reçues d'un client existant
				else
					this->reciveNewData(fds[i].fd);
			}
		}
	}
	// Fermeture propre de tous les sockets
	close_fds();
}

void Server::set_sever_socket()
{
	int en = 1;
	// Configuration de l'adresse : IPv4, toutes interfaces, port spécifié
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = INADDR_ANY;
	add.sin_port = htons(port);
	// Création du socket TCP
	server_fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fdsocket == -1)
		throw(std::runtime_error("faild to create socket"));
	// SO_REUSEADDR : permet de réutiliser l'adresse immédiatement après fermeture
	if(setsockopt(server_fdsocket, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	// O_NONBLOCK : socket non-bloquant pour le multiplexage avec poll()
	 if (fcntl(server_fdsocket, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	// Liaison du socket à l'adresse
	if (bind(server_fdsocket, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	// Mise en écoute avec file d'attente maximale
	if (listen(server_fdsocket, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));
	// Ajout du socket serveur à la liste de surveillance poll()
	new_cli.fd = server_fdsocket;
	new_cli.events = POLLIN;   // Surveiller les données entrantes
	new_cli.revents = 0;
	fds.push_back(new_cli);
}

void Server::accept_new_client()
{
	Client cli;
	// Initialisation de la structure d'adresse client
	memset(&cliadd, 0, sizeof(cliadd));
	socklen_t len = sizeof(cliadd);
	// Acceptation de la connexion entrante
	int incofd = accept(server_fdsocket, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return;
	}
	// Configuration du socket client en non-bloquant
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cout << "fcntl() failed" << std::endl;
		return;
	}
	// Préparation de l'entrée poll() pour ce client
	new_cli.fd = incofd;
	new_cli.events = POLLIN;
	new_cli.revents = 0;
	// Configuration de l'objet Client
	cli.SetFd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));  // Conversion IP en string
	// Ajout aux listes de surveillance
	clients.push_back(cli);
	fds.push_back(new_cli);
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

/**
 * @brief Reçoit et traite les données d'un client
 * @param fd File descriptor du client émetteur
 * 
 * Accumule les données dans un buffer jusqu'à réception d'un \r\n,
 * puis parse et exécute chaque commande reçue.
 */
void Server::reciveNewData(int fd)
{
	std::vector<std::string> cmd;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client *cli = GetClient(fd);
	// Réception des données du client
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	// Déconnexion ou erreur
	if(bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		// Nettoyage : retrait de tous les canaux et suppression du client
		RmChannels(fd);
		RemoveClient(fd);
		RemoveFds(fd);
		close(fd);
	}
	else
	{ 
		// Accumulation des données dans le buffer client
		cli->setBuffer(buff);
		// Attente d'une commande complète (terminée par \r\n)
		if(cli->getBuffer().find_first_of("\r\n") == std::string::npos)
			return;
		// Découpage du buffer en commandes individuelles
		cmd = split_recivedBuffer(cli->getBuffer());
		// Exécution de chaque commande
		for(size_t i = 0; i < cmd.size(); i++)
			this->parse_exec_cmd(cmd[i], fd);
		// Vidage du buffer après traitement
		if(GetClient(fd))
			GetClient(fd)->clearBuffer();
	}
}

//---------------//Server Methods
//---------------//Parsing Methods
std::vector<std::string> Server::split_recivedBuffer(std::string str)
{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;
	// Découpage ligne par ligne
	while(std::getline(stm, line))
	{
		// Suppression des caractères \r\n en fin de ligne
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos);
		vec.push_back(line);
	}
	return vec;
}

std::vector<std::string> Server::split_cmd(std::string& cmd)
{
	std::vector<std::string> vec;
	std::istringstream stm(cmd);
	std::string token;
	// Découpage par espaces (extraction des mots)
	while(stm >> token)
	{
		vec.push_back(token);
		token.clear();
	}
	return vec;
}

/**
 * @brief Vérifie si un client est pleinement authentifié
 * @param fd File descriptor du client
 * @return true si PASS/NICK/USER validés et connecté, false sinon
 * 
 * Note: le nom de la fonction est inversé par rapport à sa logique
 */
bool Server::notregistered(int fd)
{
	// Vérifie toutes les conditions d'authentification complète
	// Client inexistant, nick/user vides, nick placeholder ou non loggé = false
	if (!GetClient(fd) || GetClient(fd)->GetNickName().empty() || GetClient(fd)->GetUserName().empty() || GetClient(fd)->GetNickName() == "*"  || !GetClient(fd)->GetLogedIn())
		return false;
	return true;
}

/**
 * @brief Parse et exécute une commande IRC
 * @param cmd Ligne de commande reçue
 * @param fd File descriptor du client émetteur
 * 
 * Route la commande vers le handler approprié selon le protocole IRC.
 * Les commandes d'authentification (PASS/NICK/USER) sont toujours accessibles.
 * Les autres commandes nécessitent une authentification complète.
 */
void Server::parse_exec_cmd(std::string &cmd, int fd)
{
	if(cmd.empty())
		return ;
	std::vector<std::string> splited_cmd = split_cmd(cmd);
	size_t found = cmd.find_first_not_of(" \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	// Réponse PONG pour maintenir la connexion (requis par irssi)
	if(splited_cmd.size() && (splited_cmd[0] == "PING" || splited_cmd[0] == "ping"))
	{
		std::string pongReply = ":localhost PONG localhost";
		if (splited_cmd.size() > 1)
			pongReply += " :" + splited_cmd[1];
		pongReply += "\r\n";
		_sendResponse(pongReply, fd);
		return;
	}
	// Négociation CAP (capability) - requis par certains clients IRC
	if(splited_cmd.size() && (splited_cmd[0] == "CAP" || splited_cmd[0] == "cap"))
	{
		if (splited_cmd.size() > 1 && (splited_cmd[1] == "LS" || splited_cmd[1] == "ls"))
			_sendResponse(":localhost CAP * LS :\r\n", fd);
		return;
	}
	if(splited_cmd.size() && (splited_cmd[0] == "PONG" || splited_cmd[0] == "pong"))
		return;
    if(splited_cmd.size() && (splited_cmd[0] == "PASS" || splited_cmd[0] == "pass"))
        client_authen(fd, cmd);
	else if (splited_cmd.size() && (splited_cmd[0] == "NICK" || splited_cmd[0] == "nick"))
		set_nickname(cmd,fd);
	else if(splited_cmd.size() && (splited_cmd[0] == "USER" || splited_cmd[0] == "user"))
		set_username(cmd, fd);
	else if (splited_cmd.size() && (splited_cmd[0] == "QUIT" || splited_cmd[0] == "quit"))
		QUIT(cmd,fd);
	else if(notregistered(fd))
	{
		// Client authentifié : accès aux commandes de canal et messagerie
		if (splited_cmd.size() && (splited_cmd[0] == "KICK" || splited_cmd[0] == "kick"))
			KICK(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "JOIN" || splited_cmd[0] == "join"))
			JOIN(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "TOPIC" || splited_cmd[0] == "topic"))
			Topic(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "MODE" || splited_cmd[0] == "mode"))
			mode_command(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PART" || splited_cmd[0] == "part"))
			PART(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PRIVMSG" || splited_cmd[0] == "privmsg"))
			PRIVMSG(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "INVITE" || splited_cmd[0] == "invite"))
			Invite(cmd,fd);
		else if (splited_cmd.size())
			_sendResponse(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),splited_cmd[0]),fd);
	}
	else if (!notregistered(fd))
		_sendResponse(ERR_NOTREGISTERED(std::string("*")),fd);
}
//---------------//Parsing Methods