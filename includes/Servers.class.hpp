#ifndef SERVERS_CLASS_HPP
# define SERVERS_CLASS_HPP

# include "all_includes.hpp"

// Classe qui va pouvoir être instanciée plusieurs fois en fonction du nombre de serveurs qu'on veut lancer, chaque serveur possède ses propres informations
class Servers
{
	public:
		Servers();
		~Servers();

		/* Setters */
		void setListen(std::string word)					{this->_listen = word;};
		void setName(std::string word)						{this->_name = word;};
		void set_method(std::string word)					{this->_method.push_back(word);};
		void set_root(std::string word)						{this->_root = word;};
		void setError(std::string error, std::string page)	{this->_error.insert(std::pair<std::string, std::string>(error, page));};
		void set_index(std::string word)						{this->_index = word;};
		void setBody(std::string word)						{this->_body_size = word;};
		void setLocation()									{this->_locations.push_back(new Location());}; // Ajouter une nouvelle instance de la classe Location dans le vecteur "_locations"
		void set_listing(std::string word)					{this->_listing = word;};

		/* Getters */
		std::string							getListen()		{return this->_listen;};
		std::string							get_name()		{return this->_name;};
		std::vector<std::string>			get_method()		{return this->_method;};
		std::string							get_root()		{return this->_root;};
		std::map<std::string, std::string>	getError()		{return this->_error;};
		std::string							get_index()		{return this->_index;};
		std::string							get_body()		{return this->_body_size;};
		std::vector<Location*>				getLocation()	{return this->_locations;};
		std::string							get_listing()	{return this->_listing;};

		/* Functions */
		void stock_location(std::string line, int pos);
		bool check_method();
		bool check_error_page();
		bool check_root();
		bool check_index();
		bool check_listing();
		bool check_locations();
		bool check_client_size();

	private:
		std::string							_listen;
		std::string							_name;
		std::vector<std::string>			_method;
		std::string							_root;
		std::map<std::string, std::string>	_error;
		std::string							_index;
		std::string							_body_size;
		std::vector<Location*>				_locations;
		std::string							_listing;

};

#endif