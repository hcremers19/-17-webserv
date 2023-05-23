#ifndef CGI_HPP
# define CGI_HPP

# include "../webserv.hpp"

#define BUFFER_SIZE 65536 // Previously 2040

std::string	file_extention(std::string filePath);
std::string	search_exec(std::string filePath);
std::string	ft_st_to_string(size_t x);
void		new_env(char** envp, Request& req, std::vector<std::string>& my_env, Server* serv);
char**		vector_to_tab(std::vector<std::string>& vec);
std::string	exec_CGI(std::string filePath, char** envp, Request& req, Server* serv);

#endif /* CGI_HPP */