#include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Return the path to the executable in which to run the script, depending on
whether the file extension is .py (Python) or .pl (Perl)
Return an empty string if the extension is different
-------------------------------------------------------------------------------- */
std::string file_extention(std::string filePwd)
{
	size_t i = 0;

	while (filePwd[i++]);
	while (i && filePwd[i--] != '.');
	return ((!strcmp(&filePwd[i], ".py")) ? "/usr/bin/python2.7" : ((!strcmp(&filePwd[i], ".pl")) ? "/usr/bin/perl" : ""));
}

/* --------------------------------------------------------------------------------
Check if the executable can be launched with the received path
Return the path to the executable if it works, otherwise an empty string
-------------------------------------------------------------------------------- */
std::string search_exec(std::string filePwd)
{
	const std::string exec = file_extention(filePwd);

	if (exec == "")
	{
		std::cerr << "incompatible CGI-script" << std::endl;
		return ("");
	}
	return ((!access(exec.c_str(), X_OK)) ? exec : "");
}

/* --------------------------------------------------------------------------------
Create a new environment in vector form, which includes all the values of the
environment in which webserv is running + custom values related to the request
currently being processed
-------------------------------------------------------------------------------- */
void new_env(char** envp, Request& req, std::vector<std::string>& my_env, Server* serv)
{
	size_t i = 0;
	while (envp[i])
		my_env.push_back(envp[i++]);
	my_env.push_back("CONTENT_TYPE=" + req.get_header()["Content-Type"]);
	my_env.push_back("GATEWAY_INTERFACE=CGI/1.1");

	char path[124] = {0};
	my_env.push_back("PATH_TRANSLATED=" + std::string(getcwd(path, 124)));
	if (!req.get_query().empty())
		my_env.push_back("QUERY_STRING=" + req.get_query());
	if (!req.get_method().empty())
		my_env.push_back("REQUEST_METHOD=" + req.get_method());
	if (req.get_len())
		my_env.push_back("CONTENT_LENGTH=" + ft_st_to_string(req.get_len()));
	if (!req.get_protocol().empty())
		my_env.push_back("SERVER_SOFTWARE=" + req.get_protocol());

	my_env.push_back("SERVER_NAME=" + serv->get_name());
	my_env.push_back("HTTP_ACCEPT=" + req.get_header()["Accept"]);
	my_env.push_back("HTTP_ACCEPT_LANGUAGE=" + req.get_header()["Accept-Language"]);
	my_env.push_back("HTTP_USER_AGENT=" + req.get_header()["User-Agent"]);
	my_env.push_back("SCRIPT_NAME=" + req.get_url());
	my_env.push_back("HTTP_REFERER=" + req.get_header()["Referer"]);
}

/* --------------------------------------------------------------------------------
Transform the vector created in the new_env() function into a char* array
-------------------------------------------------------------------------------- */
char**	vector_to_tab(std::vector<std::string>& vec)
{
	char** tab;
	int i = 0;

	tab = (char**)malloc(sizeof(char*)* (vec.size() + 1));
	if (!tab)
	{
		perror("malloc vector_to_tab");
		exit(1);
	}

	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
		tab[i++] = (char*)(*it).c_str();
	tab[i] = 0;
	return tab;
}

/* --------------------------------------------------------------------------------
Looks very much like microshell or pipex : many lines to simply execve() a file
with the right executable in the right environment

The text generated by the executable is read and returned at the end of the
function
-------------------------------------------------------------------------------- */
std::string exec_CGI(std::string filePwd, char** envp, Request& req, Server* serv)
{
	std::string execPwd = search_exec(filePwd);
	if (execPwd == "")
	{
		std::cerr << "Bad file" << std::endl;
		return ("");
	}

	int		fdIn;
	int		fd_in[2];
	int		fd_out[2];
	char*	tab[3];
	char**	my_env;
	std::vector<std::string> env;
	pid_t	pid = fork();
	char	buff[2041] = {0};
	std::string	ret = "";
	int		i;

	tab[0] = (char*)execPwd.c_str();
	tab[1] = (char*)filePwd.c_str();
	tab[2] = 0;

	new_env(envp, req, env, serv);
	my_env = vector_to_tab(env);

	pipe(fd_in);
	pipe(fd_out);

	if (pid == -1)
	{
		perror("fork()");
		exit(1);
	}

	if (pid == 0)
	{
		close(fd_in[1]);
		close(fd_out[0]);
		if (dup2(fd_in[0], 0) == -1)
		{
			perror("dup2");
			exit(1);
		}
		if (dup2(fd_out[1], 1) == -1)
		{
			perror("dup2");
			exit(1);
		}
		execve(tab[0], tab, my_env);
		perror("execve");
		exit(1);
	}
	else
	{
		fdIn = dup(0);
		if (fdIn == -1)
		{
			perror("dup");
			exit(1);
		}
		if (dup2(fd_in[0], 0) == -1)
		{
			perror("dup2");
			exit(1);
		}
		if (!req.get_full_body().empty())
			write(fd_in[1], req.get_full_body().c_str(), req.get_len());
		close(fd_in[0]);
		close(fd_in[1]);
		waitpid(pid, 0, 0);
		if (dup2(fdIn, 0) == -1)
		{
			perror("dup2");
			exit(1);
		}
		free(my_env);
		close(fd_out[1]);
		i = read(fd_out[0], buff, 2040);
		if (i == -1)
		{
			perror("read");
			exit(1);
		}
		buff[i] = 0;
		ret += std::string(buff);
		while (i > 0)
		{
			i = read(fd_out[0], buff, 2040);
			if (i == -1)
			{
				perror("read");
				exit(1);

			}
			buff[i] = 0;
			ret += std::string(buff);
		}
		close(fd_out[0]);
		return ret;
	}
	return "";
}

/* --------------------------------------------------------------------------------
Converts a size_t numeric value to std::string.
-------------------------------------------------------------------------------- */
std::string ft_st_to_string(size_t x)
{
	std::ostringstream oss;

	oss << x;
	return (oss.str());
}
