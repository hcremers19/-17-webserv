#include "webserv.hpp"

/* --------------------------------------------------------------------------------
Constructor
-------------------------------------------------------------------------------- */
Conf::Conf()
{
	this->_directives.push_back("server");
	this->_directives.push_back("listen");
	this->_directives.push_back("server_name");
	this->_directives.push_back("allowed_methods");
	this->_directives.push_back("root");
	this->_directives.push_back("error_page");
	this->_directives.push_back("index");
	this->_directives.push_back("client_max_body_size");
	this->_directives.push_back("location");
	this->_directives.push_back("dir_listing");
	this->_directives.push_back("redir");
}

/* --------------------------------------------------------------------------------
Destructor
-------------------------------------------------------------------------------- */
Conf::~Conf()
{
	for (size_t i = 0; i < this->_servers.size(); i++)
		delete this->_servers[i];
	this->_servers.clear();
	this->_file.clear();
	this->_filePos.clear();
	this->_directives.clear();
}

/* --- ACCESSORS --- */

std::vector<Servers*>	Conf::get_servers() const
{
	return this->_servers;
}

/* --------------------------------------------------------------------------------
Ajouter une première instance de la classe "Servers" dans le vecteur "_servers"
-------------------------------------------------------------------------------- */
void	Conf::set_servers()
{
	this->_servers.push_back(new Servers());
}

/* --- MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Vérifier que toutes les données stockées dans la classe Conf sont correctes
-------------------------------------------------------------------------------- */
void	Conf::check_data()
{
	for (size_t i = 0; i < this->_servers.size(); i++)
	{
		if (this->_servers[i]->get_name().empty() || this->_servers[i]->get_listen().empty() || this->_servers[i]->get_root().empty()
			|| this->_servers[i]->get_index().empty() || this->_servers[i]->get_method().empty()  || this->_servers[i]->get_body_size().empty()
			|| this->_servers[i]->get_listing().empty())
			throw DirMissing();
		if (!this->_servers[i]->check_locations())
			throw DirMissing();
		if (this->_servers[i]->get_listen().size() > 4 || !is_digit_str(this->_servers[i]->get_listen()) || !is_digit_str(this->_servers[i]->get_body_size()))
			throw NotINT();
		if (!this->_servers[i]->check_error_page())
			throw ErrorPage();
		if (!this->_servers[i]->check_method())
			throw MethWrong();
		if (!this->_servers[i]->check_root())
			throw RootErr();
		if (!this->_servers[i]->check_index())
			throw IndexLoc();
		if (!this->_servers[i]->check_listing())
			throw ListingErr();
		if (!this->_servers[i]->check_client_size())
			throw SizeErr();
	}
}

/* --------------------------------------------------------------------------------
Vérifier chaque ligne pour savoir s'il s'agit d'une directive ou non
-------------------------------------------------------------------------------- */
void	Conf::check_directive()
{
	std::size_t len = this->_file.size();

	for (size_t i = 0; i < len; i++)
		this->is_directive(this->_file[i], i);
}

/* --------------------------------------------------------------------------------
Initier le vecteur "_filePos" qui reprend la position de la directive, s'il
s'agit de location ou server
-------------------------------------------------------------------------------- */
void	Conf::init_file_pos()
{
	//0 = server; 1 = location;
	size_t len =this->_file.size(), pos = 0;
	std::string word;

	for (size_t i = 0; i < len; i++)
	{
		word = ft_first_word(this->_file[i]);
		if (i == 0 && word != "server")
			throw DirMissing();
		if (word == "server")
			pos = 0;
		else if (word == "location")
			pos = 1;
		this->_filePos.push_back(pos);
	}
}

/* --------------------------------------------------------------------------------
Déterminer si la ligne passée en argument est une directive ou non et si elle
est correctement formatée
-------------------------------------------------------------------------------- */
void	Conf::is_directive(std::string line, int pos)
{
	std::size_t count = count_words(line), len = this->_directives.size();
	std::string word = ft_first_word(line);

	for (size_t i = 0; i < len; i++)
	{
		if (word == this->_directives[i] || word == "{" || word == "}" || word == "["|| word == "]")
		{
			if ((count == 1 && word != "server" && word != "{" && word != "}" && word != "[" && word != "]") || (count == 2 && word == "error_page"))
				throw MissingArgv();
			else if (count >= 3 && word != "error_page")
				throw TooMuchArgv();
			else if ((this->_filePos[pos] == 1 && (word == "listen" || word == "client_max_body_size" || word == "server_name")) || (this->_filePos[pos] == 0 && (word == "redir"))) // 0 == server, 1 == location
				throw DirWrongPlace();
			return;
		}
	}
	throw DirWrong();
}

/* --------------------------------------------------------------------------------
Lire le fichier passé en argument et passer chaque ligne dans une entrée du vec-
teur "_file"
-------------------------------------------------------------------------------- */
void	Conf::read_file(std::string name)
{
	std::ifstream file(name);
	std::string output;

	while (std::getline(file, output))
	{
		std::size_t len = output.length();

		for (std::size_t i = 0; i < len; i++)
		{
			if (!isspace(output[i]))
			{
				this->_file.push_back(output);
				break;
			}
		}
	}
	if (this->_file.empty())
		throw DirMissing();
}

/* --------------------------------------------------------------------------------
Stocker toutes les informations du fichier de configuration dans les variables
appropriées
-------------------------------------------------------------------------------- */
void	Conf::stock_data()
{
	int len = this->_file.size(), 
		nb_server = -1, 
		nb_locations = -1,
		status = 0,
		status1 = 0;

	for (int i = 0; i < len; i++)
	{
		if (this->_filePos[i] == 0)
		{
			if (ft_first_word(this->_file[i]) == "server")
			{
				int open = find_char(this->_file, '{',i);
				int close = find_char(this->_file, '}', open);
				int new_open = find_char(this->_file, '{', open + 1);
				if (open == -1 || close == -1)
					throw DirMissing();
				else if (open != i + 1)
					throw DirMissing();
				else if (new_open != -1 && new_open < close)
					throw DirMissing();
				
				this->set_servers(); // New server
				nb_server++;
				nb_locations = -1;
				status = 0;
			}
			else if (ft_first_word(this->_file[i]) == "}")
				status = 1;
			else if (!status)
				this->stock_server(this->_file[i], this->_servers[nb_server]);
		}
		else
		{
			if (ft_first_word(this->_file[i]) == "location")
			{
				int open1 = find_char(this->_file, '[',i);
				int close1 = find_char(this->_file, ']', open1);
				int new_open1 = find_char(this->_file, '[', open1 + 1);
				if (open1 == -1 || close1 == -1)
					throw DirMissing();
				else if (open1 != i + 1)
					throw DirMissing();
				else if (new_open1 != -1 && new_open1 < close1)
					throw DirMissing();
				this->_servers[nb_server]->set_location();
				nb_locations++;
				status1 = 0;
			}
			if (ft_first_word(this->_file[i]) == "]")
				status1 = 1;
			else if (!status1)
				this->_servers[nb_server]->stock_location(this->_file[i], nb_locations);
		}
	}
}

/* --------------------------------------------------------------------------------
Stocker les informations de la ligne "line" dans le serveur "server"
-------------------------------------------------------------------------------- */
void	Conf::stock_server(std::string line, Servers* server)
{
	std::size_t count = count_words(line);
	std::string word = ft_first_word(line), last;

	if (count == 2)
	{
		last = ft_last_word(line);
		std::map<std::string, std::string> settings = {
			{"listen", server->get_listen()},
			{"server_name", server->get_name()},
			{"root", server->get_root()},
			{"index", server->get_index()},
			{"client_max_body_size", server->get_body_size()},
			{"dir_listing", server->get_listing()}
		};

		if (settings.find(word) != settings.end())
		{
			if (!settings[word].empty())
				throw DirTwice();
			settings[word] = last;
			if (word == "listen")
				server->set_listen(last);
			else if (word == "server_name")
				server->set_name(last);
			else if (word == "root")
				server->set_root(last);
			else if (word == "index")
				server->set_index(last);
			else if (word == "client_max_body_size")
				server->set_body_size(last);
			else if (word == "dir_listing")
				server->set_listing(last);
		}
		else if (word == "allowed_methods")
			server->set_method(last);
	}
	else if (count >= 3)
	{
		std::stringstream ss(line);
		std::string token;
		last = ft_last_word(line);

		while (ss >> token)
			if (line.find(token) != line.rfind(last) && token != word)
				server->set_error(token, last);
	}

}

/* --- NON-MEMBER FUNCTION --- */

/* --------------------------------------------------------------------------------
Vérifier si la string "word" est composée uniquement d'ints ou pas
-------------------------------------------------------------------------------- */
bool	is_digit_str(std::string word)
{
	int i = -1;
	while (word[++i])
		if (!isdigit(word[i]))
			return false;
	return true;
}

/* --------------------------------------------------------------------------------
Compter le nombre de mots dans la ligne "sentence"
-------------------------------------------------------------------------------- */
int		count_words(std::string sentence)
{
	int ret = 0, i = -1;

	while (sentence[++i + 1])
		if (!isspace(sentence[i]) && isspace(sentence[i + 1]))
			ret++;
	if (!isspace(sentence[i]))
		ret++;
	return (ret);
}

/* --------------------------------------------------------------------------------
Fonction non-membre qui permet de retrouver le caractère "c" dans toutes les
strings du vecteur "file", à partir de la position "start"
-------------------------------------------------------------------------------- */
int 	find_char(std::vector<std::string> file, char c, int start = 0)
{
	int len = file.size();
	std::string str;

	for (int i = start; i < len; i++)
	{
		str = file[i];
		for (int y = 0; y < (int)str.size(); y++)
			if (str[y] == c)
				return (i);
	}
	return (-1);
}

/* --------------------------------------------------------------------------------
Retourner le premier mot de la ligne "line"
-------------------------------------------------------------------------------- */
std::string ft_first_word(std::string line)
{
	int i = 0, j;
	while (isspace(line[i]) && line[i++]);
	j = i - 1;
	while (!isspace(line[++j]) && line[j]);
	return (line.substr(i, j - i));
}

/* --------------------------------------------------------------------------------
Retourner le dernier mot de la ligne "line"
-------------------------------------------------------------------------------- */
std::string ft_last_word(std::string line)
{
	int i = line.length() - 1, j;
	while (isspace(line[--i]) && line[i]);

	j = i + 1;
	while (--j > 0 && !isspace(line[j]));

	return (line.substr(j + 1, i - j));
}