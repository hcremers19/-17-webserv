#include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Constructor

Reçoit la requête sous forme de chaîne de caractères et parse ses différentes informations pour initialiser les différentes variables de la classe
-------------------------------------------------------------------------------- */
Request::Request(char *requete)
{
	// std::ofstream MyFile("my_input.txt");
	// MyFile << requete;
	// MyFile.close();
	this->_charRequest = requete;
	std::stringstream ss(requete);
	this->_request = requete;
	this->_len = 0;
	ss >> this->_method >> this->_url >> this->_protocol;
	this->make_query();
	if (this->_method == "GET")
		this->make_GET(ss);
	else if (this->_method == "POST")
		this->make_POST(ss);
}

/* --------------------------------------------------------------------------------
Destructor
-------------------------------------------------------------------------------- */
Request::~Request()
{
	this->_header.clear();
}

/* --- MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Vérifier que la méthode est correcte (GET, POST ou DELETE)
-------------------------------------------------------------------------------- */
int Request::check_method()
{
	return ((this->_method != "POST" && this->_method != "GET" && this->_method != "DELETE") ? 405 : ((this->_protocol != "HTTP/1.1") ? 505 : -1));
}

/* --------------------------------------------------------------------------------
Pas compris

Décortiquer la méthode POST ?

Coder soi-même les fonctions c++11
S'assurer qu'on est ok avec les opérateurs >>
-------------------------------------------------------------------------------- */
void Request::make_POST(std::stringstream& ss)
{
	std::string token, line, key;
	std::string buff;
	size_t pos = this->_request.find("\r\n\r\n");

	while (ss >> token)
	{
		// std::cout << "Token = " << token << std::endl;
		if (token.find("boundary=") != std::string::npos)
			this->_boundary = token.substr(token.find("boundary=") + 9);
		if (token == "Content-Length:")
		{
			if (!key.empty() && !line.empty() && key != token)
			{
				if (ft_back(line) == ' ')
					ft_pop_back(line); //remove space
				this->_header.insert(std::pair<std::string, std::string>(key, line));
				line.clear();
			}
			key = token;
			ss >> token;
			this->_header.insert(std::pair<std::string, std::string>(key, token));
			this->_len = atoi(token.c_str());
			key.clear();
		}
		else if (this->_request.find(token) >= pos - token.length())
		{
			pos += 4;
			if (!key.empty() && key != token)
			{
				if (!line.empty())
					ft_pop_back(line); //remove space
				this->_header.insert(std::pair<std::string, std::string>(key, (line.empty()) ? token : line));
			}
			size_t pos_header = pos;
			while (pos < this->_len + pos_header) // || _request[pos + 1]
				this->_fullBody += this->_charRequest[pos++];
			if (this->_boundary.empty())
				this->_body = this->_fullBody;
			break;
		}
		else if (ft_back(token) == ':')
		{
			if (!key.empty() && key != token)
			{
				if (ft_back(line) == ' ')
					ft_pop_back(line);
				this->_header.insert(std::pair<std::string, std::string>(key, line));
			}
			ft_pop_back(token);
			key = token;
			line.clear();
		}
		else
		{
			line += ((line.empty()) ? "" : " ") + token;
		}
	}
}

/* --------------------------------------------------------------------------------
Pas compris

Décortiquer la requête GET ?
-------------------------------------------------------------------------------- */
void Request::make_GET(std::stringstream& ss)
{
	std::string token, line, key;

	while (ss >> token)
	{
		if (ft_back(token) == ':')
		{
			if (!key.empty() && !line.empty() && key != token)
			{
				if (ft_back(line) == ' ')
					ft_pop_back(line);
				this->_header.insert(std::pair<std::string, std::string>(key, line));
			}
			ft_pop_back(token);
			key = token;
			line.clear();
		}
		else
			line += ((line.empty()) ? "" : " ") + token;
	}
	if (!line.empty() && !key.empty())
	{
		if (ft_back(line) == ' ')
			ft_pop_back(line);
		this->_header.insert(std::pair<std::string, std::string>(key, line));
	}
}

/* --------------------------------------------------------------------------------
Isoler la query
-------------------------------------------------------------------------------- */
void Request::make_query()
{
	size_t pos = this->_url.find("?");
	if (pos != std::string::npos)
		this->_query= this->_url.substr(pos + 1);
}

/* --- NON MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Reproduire le fonctionnement de la fonction C++11 std::string.back()
-------------------------------------------------------------------------------- */
char&	ft_back(std::string& str)
{
	return (str[str.size() - 1]);
}

/* --------------------------------------------------------------------------------
Reproduire le fonctionnement de la fonction C++11 std::string.pop_back()
-------------------------------------------------------------------------------- */
void	ft_pop_back(std::string& str)
{
	if (!str.empty())
    	str.resize(str.size() - 1);
}
