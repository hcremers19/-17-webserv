#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Gets the data from the parsing, everything that is configured is found here
-------------------------------------------------------------------------------- */
class Config
{
	public:
		Config();
		~Config();

		std::vector<Server*>	get_servers() const;

		void	check_data();
		void	check_directive();
		void	init_file_pos();
		void	read_file(std::string name);
		void	store_data();

	private:
		void	set_servers();
		void	store_server(std::string line, Server* server);
		void	is_directive(std::string line, int pos);

		std::vector<Server*>		_servers;
		std::vector<std::string>	_file;
		std::vector<int>			_filePos;
		std::vector<std::string>	_directives;
};

bool 		is_digit_str(std::string word);
int			count_words(std::string sentence);
int 		find_char(std::vector<std::string> file, char c, int start);
std::string ft_first_word(std::string line);
std::string	ft_last_word(std::string line);

#endif /* CONFIG_HPP */