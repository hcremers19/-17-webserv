#ifndef LOCATION_CLASS_HPP
# define LOCATION_CLASS_HPP

# include "all_includes.hpp"

// Classe dans laquelle seront stockées les informations relatives à la directive location (expliquer)
class Location
{
	public:
		Location();
		~Location();

		void set_dir(std::string word);
		void set_root(std::string word);
		void set_index(std::string word);
		void set_method(std::string word);
		void set_listing(std::string word);
		void set_redir(std::string word);

		std::string					get_dir();
		std::vector<std::string>	get_method();
		std::string					get_root();
		std::string					get_index();
		std::string					get_listing();
		std::string					get_redir();

	private:
		std::string					_dir;
		std::vector<std::string>	_method;
		std::string					_root;
		std::string					_index;
		std::string					_listing;
		std::string					_redir;
};

#endif