#ifndef CONF_CLASS_HPP
# define CONF_CLASS_HPP

# include "all_includes.hpp"

// Récupère les données du parsing, tout ce qui est configuré se retrouve ici 
class Conf
{
	public:
		Conf();
		~Conf();

		std::vector<Servers*>	get_servers() const;

		void	check_data();
		void	check_directive();
		void	init_file_pos();
		void	read_file(std::string name);
		void	stock_data();

	private:
		void	set_servers();
		void	stock_server(std::string line, Servers* server);
		void	is_directive(std::string line, int pos);

		std::vector<Servers*>		_servers;
		std::vector<std::string>	_file;
		std::vector<int>			_filePos;
		std::vector<std::string>	_directives;
};

int 	find_char(std::vector<std::string> file, char c, int start);

#endif