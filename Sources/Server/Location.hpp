#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Class in which the information related to the location directive will be stored
-------------------------------------------------------------------------------- */
class Location
{
	public:
		Location();
		~Location();

		void set_dir(std::string word);
		void set_index(std::string word);
		void set_listing(std::string word);
		void set_method(std::string word);
		void set_redir(std::string word);
		void set_root(std::string word);

		std::string					get_dir();
		std::string					get_index();
		std::string					get_listing();
		std::vector<std::string>	get_method();
		std::string					get_redir();
		std::string					get_root();

	private:
		std::string					_dir;
		std::string					_index;
		std::string					_listing;
		std::vector<std::string>	_method;
		std::string					_redir;
		std::string					_root;
};

#endif /* LOCATION_HPP */