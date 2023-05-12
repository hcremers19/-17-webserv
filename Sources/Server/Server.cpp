#include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Constructor
-------------------------------------------------------------------------------- */
Server::Server()
{
}

/* --------------------------------------------------------------------------------
Destructor

Vide les différents conteneurs qui ont été utilisés dans cette classe
-------------------------------------------------------------------------------- */
Server::~Server()
{
	for (size_t i = 0; i < this->_locations.size(); i++)
		delete this->_locations[i];
	this->_locations.clear();
	this->_method.clear();
	this->_error.clear();
}

/* --- MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Vérifier que la taille max du serveur (directive client_max_body_size, variable _bodySize) n'est pas supérieure à 2147483647 ou inférieure à 10
-------------------------------------------------------------------------------- */
bool Server::check_client_size()
{
	return (this->_bodySize.size() > 10) ? false : (atoi(this->_bodySize.c_str()) <= 2147483647);
}

/* --------------------------------------------------------------------------------
Vérifier que les données stockées dans les instances de la classe Location reprises dans le vecteur membre "_locations" ne sont pas vides
-------------------------------------------------------------------------------- */
bool Server::check_locations()
{
	for (size_t i = 0; i < this->_locations.size(); i++)
		if (this->_locations[i]->get_dir().empty() || this->_locations[i]->get_method().empty() || this->_locations[i]->get_root().empty() || this->_locations[i]->get_index().empty() || this->_locations[i]->get_listing().empty())
			return false;
	return true;
}

/* --------------------------------------------------------------------------------
Vérifier que toutes les locations ont bien un index
-------------------------------------------------------------------------------- */
bool Server::check_index()
{
	if (this->_index.find(".html") == std::string::npos)
		return false;
	for (size_t i = 0; i < this->_locations.size(); i++)
	{
		if (this->_locations[i]->get_index().empty())
			return false;
		else if (this->_locations[i]->get_index().find(".html") == std::string::npos)
			return false;
	}
	return true;
}

/* --------------------------------------------------------------------------------
Vérifier si le contenu de la variable _listing (dir_listing) est bien "on" ou "off"
-------------------------------------------------------------------------------- */
bool Server::check_listing()
{
	if (this->_listing != "on" && this->_listing != "off")
		return false;
	for (size_t i = 0; i < this->_locations.size(); i++)
		if (this->_locations[i]->get_listing() != "on" && this->_locations[i]->get_listing() != "off")
			return false;
	return true;
}

/* --------------------------------------------------------------------------------
Vérifier que les méthodes autorisées sont POST, GET ou DELETE dans un serveur ou dans ses locations
-------------------------------------------------------------------------------- */
bool Server::check_method()
{
	for (size_t j = 0; j < this->_method.size(); j++)
		if (this->_method[j] != "POST" &&  this->_method[j] != "GET" &&  this->_method[j] != "DELETE")
			return false;
	for (size_t i = 0; i < this->_locations.size(); i++)
		for (size_t j = 0; j < this->_locations[i]->get_method().size(); j++)
			if (this->_locations[i]->get_method()[j] != "POST" && this->_locations[i]->get_method()[j] != "GET" && this->_locations[i]->get_method()[j] != "DELETE")
				return false;
	return true;
}

/* --------------------------------------------------------------------------------
Vérifier que les éléments du map "_error" (les pages d'erreur) sont bien formatés (numéro d'erreur et extension .html)
-------------------------------------------------------------------------------- */
bool Server::check_error_page()
{
	std::map<std::string, std::string>::iterator it = this->_error.begin();
	std::map<std::string, std::string>::iterator it_end = this->_error.end();
	while (it != it_end)
	{
		if (!is_digit_str(it->first) || (it->second).find(".html") == std::string::npos)
			return false;
		it++;
	}
	return true;
}

/* --------------------------------------------------------------------------------
Vérifier que le contenu de la variable _root est correctement formatée
-------------------------------------------------------------------------------- */
bool Server::check_root()
{
	if (this->_root.length() <= 2)
		return (this->_root == "./");
	if (this->_root[this->_root.length() - 1] != '/')
		this->_root += "/";
	for (size_t i = 0; i < this->_locations.size(); i++)
	{
		if (this->_locations[i]->get_root().length() <= 2)
		{
			if (this->_locations[i]->get_root() == "./")
				continue;
			return false;
		}
		if (this->_locations[i]->get_root()[this->_locations[i]->get_root().length() - 1] != '/')
			this->_locations[i]->set_root(this->_locations[i]->get_root() + "/");
	}
	return true;
}

/* --------------------------------------------------------------------------------
Stocker les informations de la ligne "line" dans le vecteur membre "_locations" à la position "pos"
-------------------------------------------------------------------------------- */
void Server::stock_location(std::string line, int pos)
{
	std::string word = ft_first_word(line);
	std::string last = ft_last_word(line);

	if (word == "location")
		this->_locations[pos]->set_dir(last);
	else if (word == "root")
	{
		if (!this->_locations[pos]->get_root().empty())
			throw DirTwice();
		this->_locations[pos]->set_root(last);
	}
	else if (word == "index")
	{
		if (!this->_locations[pos]->get_index().empty())
			throw DirTwice();
		this->_locations[pos]->set_index(last);
	}
	else if (word == "allowed_methods")
		this->_locations[pos]->set_method(last);
	else if (word == "dir_listing")
	{
		if (!this->_locations[pos]->get_listing().empty())
			throw DirTwice();
		this->_locations[pos]->set_listing(last);
	}
	else if (word == "redir")
	{
		if (!this->_locations[pos]->get_redir().empty())
			throw DirTwice();
		this->_locations[pos]->set_redir(last);
	}
}
