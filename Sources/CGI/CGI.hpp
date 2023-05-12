#ifndef CGI_HPP
# define CGI_HPP

# include "../webserv.hpp"

char**		vector_to_tab(std::vector<std::string>& vec);
std::string	exec_CGI(std::string filePwd, char** envp, Request& req, Server* serv);
std::string	file_extention(std::string filePwd);
std::string	search_exec(std::string filePwd);
void		new_env(char** envp, Request& req, std::vector<std::string>& my_env, Server* serv);
std::string ft_st_to_string(size_t x);

#endif /* CGI_HPP */