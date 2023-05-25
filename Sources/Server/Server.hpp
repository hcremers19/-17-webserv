#ifndef SERVER_HPP
# define SERVER_HPP

# include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Class that contains all the information related to a single server, so it can be
instantiated several times if the configuration file requires several servers
-------------------------------------------------------------------------------- */
class Server
{
	public:
		Server();
		~Server();

		void set_port(std::string word)						{this->_port = word;}
		void set_name(std::string word)						{this->_name = word;}
		void set_method(std::string word)					{this->_method.push_back(word);}
		void set_root(std::string word)						{this->_root = word;}
		void set_error(std::string error, std::string page)	{this->_error.insert(std::pair<std::string, std::string>(error, page));}
		void set_index(std::string word)					{this->_index = word;}
		void set_body_size(std::string word)				{this->_bodySize = word;}
		void set_location()									{this->_locations.push_back(new Location());}
		void set_listing(std::string word)					{this->_listing = word;}

		std::string							get_port()		{return this->_port;}
		std::string							get_name()		{return this->_name;}
		std::vector<std::string>			get_method()	{return this->_method;}
		std::string							get_root()		{return this->_root;}
		std::map<std::string, std::string>	get_error()		{return this->_error;}
		std::string							get_index()		{return this->_index;}
		std::string							get_body_size()	{return this->_bodySize;}
		std::vector<Location*>				get_location()	{return this->_locations;}
		std::string							get_listing()	{return this->_listing;}

		void stock_location(std::string line, int pos);
		bool check_method();
		bool check_error_page();
		bool check_root();
		bool check_index();
		bool check_listing();
		bool check_locations();
		bool check_client_size();

	private:
		std::string							_port;
		std::string							_name;
		std::vector<std::string>			_method;
		std::string							_root;
		std::map<std::string, std::string>	_error;
		std::string							_index;
		std::string							_bodySize;
		std::vector<Location*>				_locations;
		std::string							_listing;

};

#endif /* SERVER_HPP */