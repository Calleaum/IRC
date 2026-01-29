#include "../INC/Channel.hpp"

/* ************************************************************************** */
/*                         CONSTRUCTORS / DESTRUCTOR                          */
/* ************************************************************************** */

/**
 * @brief Constructeur par défaut - initialise un canal vide
 * 
 * Modes disponibles :
 *   i - invite-only    : seuls les invités peuvent rejoindre
 *   t - topic protected: seuls les opérateurs peuvent changer le topic
 *   k - key            : mot de passe requis pour rejoindre
 *   o - operator       : privilèges d'opérateur
 *   l - limit          : limite du nombre d'utilisateurs
 */
Channel::Channel()
{
	this->invit_only = 0;
	this->topic = 0;
	this->key = 0;
	this->limit = 0;
	this->topic_restriction = false;
	this->name = "";
	this->topic_name = "";
	char charaters[5] = {'i', 't', 'k', 'o', 'l'};
	for(int i = 0; i < 5; i++)
		modes.push_back(std::make_pair(charaters[i],false));
	this->created_at = "";
}

Channel::~Channel()
{}

Channel::Channel(Channel const &src)
{*this = src;}

Channel &Channel::operator=(Channel const &src)
{
	if (this != &src)
	{
		this->invit_only = src.invit_only;
		this->topic = src.topic;
		this->key = src.key;
		this->limit = src.limit;
		this->topic_restriction = src.topic_restriction;
		this->name = src.name;
		this->password = src.password;
		this->created_at = src.created_at;
		this->topic_name = src.topic_name;
		this->clients = src.clients;
		this->admins = src.admins;
		this->modes = src.modes;
	}
	return *this;
}
//---------------//Setters
void Channel::SetInvitOnly(int invit_only){this->invit_only = invit_only;}
void Channel::SetTopic(int topic){this->topic = topic;}
void Channel::SetTime(std::string time){this->time_creation = time;}
void Channel::SetKey(int key){this->key = key;}
void Channel::SetLimit(int limit){this->limit = limit;}
void Channel::SetTopicName(std::string topic_name){this->topic_name = topic_name;}
void Channel::SetPassword(std::string password){this->password = password;}
void Channel::SetName(std::string name){this->name = name;}
void Channel::set_topicRestriction(bool value){this->topic_restriction = value;}
void Channel::setModeAtindex(size_t index, bool mode){modes[index].second = mode;}

void Channel::set_createiontime()
{
	std::time_t _time = std::time(NULL);
	std::ostringstream oss;
	oss << _time;
	this->created_at = std::string(oss.str());
}
//---------------//Setters
//---------------//Getters
int Channel::GetInvitOnly(){return this->invit_only;}
int Channel::GetTopic(){return this->topic;}
int Channel::GetKey(){return this->key;}
int Channel::GetLimit(){return this->limit;}

int Channel::GetClientsNumber()
{
	return this->clients.size() + this->admins.size();
}

bool Channel::Gettopic_restriction() const
{return this->topic_restriction;}

bool Channel::getModeAtindex(size_t index)
{return modes[index].second;}

/**
 * @brief Vérifie si un utilisateur est membre du canal
 * @param nick Nickname de l'utilisateur à rechercher
 * @return true si présent (client ou admin), false sinon
 */
bool Channel::clientInChannel(std::string &nick){
	// Recherche dans la liste des clients normaux
	for(size_t i = 0; i < clients.size(); i++)
	{
		if(clients[i].GetNickName() == nick)
			return true;
	}
	// Recherche dans la liste des administrateurs
	for(size_t i = 0; i < admins.size(); i++)
	{
		if(admins[i].GetNickName() == nick)
			return true;
	}
	return false;
}

std::string Channel::GetTopicName(){return this->topic_name;}
std::string Channel::GetPassword(){return this->password;}
std::string Channel::GetName(){return this->name;}
std::string Channel::GetTime(){return this->time_creation;}
std::string Channel::get_creationtime(){return created_at;}

std::string Channel::getModes()
{
	std::string mode;
	// Parcours des modes actifs (sauf 'o' qui est par utilisateur)
	for(size_t i = 0; i < modes.size(); i++)
	{
		if(modes[i].first != 'o' && modes[i].second)
			mode.push_back(modes[i].first);
	}
	// Préfixe '+' si au moins un mode est actif
	if(!mode.empty())
		mode.insert(mode.begin(),'+');
	return mode;
}

std::string Channel::clientChannel_list()
{
	std::string list;
	// Ajout des admins avec préfixe '@' (opérateur)
	for(size_t i = 0; i < admins.size(); i++)
	{
		list += "@" + admins[i].GetNickName();
		if((i + 1) < admins.size())
			list += " ";
	}
	// Séparateur entre admins et clients
	if(clients.size())
		list += " ";
	// Ajout des clients normaux (sans préfixe)
	for(size_t i = 0; i < clients.size(); i++)
	{
		list += clients[i].GetNickName();
		if((i + 1) < clients.size())
			list += " ";
	}
	return list;
}

Client *Channel::get_client(int fd)
{
	// Recherche d'un client par son file descriptor
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->GetFd() == fd)
			return &(*it);
	}
	return NULL;
}

Client *Channel::get_admin(int fd)
{
	// Recherche d'un admin par son file descriptor
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it)
	{
		if (it->GetFd() == fd)
			return &(*it);
	}
	return NULL;
}

Client* Channel::GetClientInChannel(std::string name)
{
	// Recherche par nickname dans les clients
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->GetNickName() == name)
			return &(*it);
	}
	// Recherche par nickname dans les admins
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it)
	{
		if (it->GetNickName() == name)
			return &(*it);
	}
	return NULL;
}
//---------------//Getters
//---------------//Methods
void Channel::add_client(Client newClient){clients.push_back(newClient);}
void Channel::add_admin(Client newClient){admins.push_back(newClient);}

void Channel::remove_client(int fd)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->GetFd() == fd)
		{
			clients.erase(it);
			break;
		}
	}
}
void Channel::remove_admin(int fd)
{
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it){
		if (it->GetFd() == fd)
		{
			admins.erase(it);
			break;
		}
	}
}
bool Channel::change_clientToAdmin(std::string& nick){
	size_t i = 0;
	// Recherche du client par nickname
	for(; i < clients.size(); i++)
	{
		if(clients[i].GetNickName() == nick)
			break;
	}
	// Si trouvé : transfert vers la liste des admins
	if(i < clients.size())
	{
		admins.push_back(clients[i]);
		clients.erase(i + clients.begin());
		return true;
	}
	return false;
}

bool Channel::change_adminToClient(std::string& nick){
	size_t i = 0;
	// Recherche de l'admin par nickname
	for(; i < admins.size(); i++)
	{
		if(admins[i].GetNickName() == nick)
			break;
	}
	// Si trouvé : rétrogradation vers la liste des clients
	if(i < admins.size())
	{
		clients.push_back(admins[i]);
		admins.erase(i + admins.begin());
		return true;
	}
	return false;

}

/* ************************************************************************** */
/*                              BROADCAST METHODS                             */
/* ************************************************************************** */

/**
 * @brief Envoie un message à tous les membres du canal
 * @param rpl1 Message à diffuser
 */
void Channel::sendTo_all(std::string rpl1)
{
	// Envoi aux administrateurs du canal
	for(size_t i = 0; i < admins.size(); i++)
		if(send(admins[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
			std::cerr << "send() faild" << std::endl;
	// Envoi aux clients normaux du canal
	for(size_t i = 0; i < clients.size(); i++)
		if(send(clients[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
			std::cerr << "send() faild" << std::endl;
}

/**
 * @brief Envoie un message à tous les membres sauf l'expéditeur
 * @param rpl1 Message à diffuser
 * @param fd File descriptor de l'expéditeur à exclure
 */
void Channel::sendTo_all(std::string rpl1, int fd)
{
	// Envoi aux administrateurs (sauf l'expéditeur)
	for(size_t i = 0; i < admins.size(); i++)
	{
		if(admins[i].GetFd() != fd)
			if(send(admins[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
				std::cerr << "send() faild" << std::endl;
	}
	// Envoi aux clients normaux (sauf l'expéditeur)
	for(size_t i = 0; i < clients.size(); i++)
	{
		if(clients[i].GetFd() != fd)
			if(send(clients[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
				std::cerr << "send() faild" << std::endl;
	}
}