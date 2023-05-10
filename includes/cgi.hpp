#ifndef CGI_HPP
# define CGI_HPP

# include "all_includes.hpp"

char**		vector_to_tab(std::vector<std::string>& vec);
std::string	exec_CGI(std::string filePwd, char** envp, Requete& req, Servers* serv);
std::string	file_extention(std::string filePwd);
std::string	search_exec(std::string filePwd, char** envp);
void		new_env(char** envp, Requete& req, std::vector<std::string>& my_env, Servers* serv);

#endif