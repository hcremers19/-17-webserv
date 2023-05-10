#include "all_includes.hpp"

/* --------------------------------------------------------------------------------
Renvoyer le chemin vers l'exécutable dans lequel lancer le script, en fonction de si l'extension du fichier est .py (Pyhton) ou .pl (Perl)
Renvoyer une string vide si l'extension est différente
-------------------------------------------------------------------------------- */
std::string file_extention(std::string filePwd)
{
	size_t  i = 0;

	while (filePwd[i++]);
	while (i && filePwd[i--] != '.');
	return ((!strcmp(&filePwd[i], ".py")) ? "/usr/bin/python2.7" : ((!strcmp(&filePwd[i], ".pl")) ? "/usr/bin/perl" : ""));
}

/* --------------------------------------------------------------------------------
Vérifier si l'exécutable peut être lancé avec le chemin reçu
(SUPPRIMER LE PARAMÈTRE ENVP QUI N'A PLUS L'AIR D'ÊTRE NÉCESSAIRE)
Renvoyer le chemin vers l'exécutable s'il fonctionne, sinon une chaîne vide
-------------------------------------------------------------------------------- */
std::string search_exec(std::string filePwd, char** envp)
{
	(void)envp;
	const std::string exec = file_extention(filePwd);

	if (exec == "")
	{
		std::cerr << "incompatible CGI-script" << std::endl;
		return ("");
	}
	return ((!access(exec.c_str(), X_OK)) ? exec : "");
}

/* --------------------------------------------------------------------------------
Attention, ce my_env n'eest pas le même que celui de la fonction exec_cgi
Créer un nouvel environnement sous forme de vecteur, qui reprend toutes les valeurs de l'environnement dans lequel webserv est exécuté + des valeurs personnalisées relatives à la requête traitée actuellement
-------------------------------------------------------------------------------- */
void new_env(char** envp, Requete& req, std::vector<std::string>& my_env, Servers* serv)
{
	size_t  i = 0;
	while (envp[i])
		my_env.push_back(envp[i++]);
	my_env.push_back("CONTENT_TYPE=" + req.get_header()["Content-Type"]);
	my_env.push_back("GATEWAY_INTERFACE=CGI/1.1");

	char path[124] = {0};
	my_env.push_back("PATH_TRANSLATED=" + std::string(getcwd(path, 124)));
	if (!req.get_query().empty())
		my_env.push_back("QUERY_STRING=" + req.get_query()); // getQS pour querry string
	if (!req.get_method().empty())
		my_env.push_back("REQUEST_METHOD=" + req.get_method());
	if (req.get_len())
		my_env.push_back("CONTENT_LENGTH=" + std::to_string(req.get_len()));
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
Transformer le vecteur créé dans la fonction new_env en un tableau de char*
-------------------------------------------------------------------------------- */
char**	vector_to_tab(std::vector<std::string>& vec)
{
	char **tab;
	int i = 0;

	tab = (char** )malloc(sizeof(char *)* (vec.size() + 1));
	if (!tab)
	{
		perror("malloc vector_to_tab");
		exit(1);
	}

	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
		tab[i++] = (char *)(*it).c_str();
	tab[i] = 0;
	return tab;
}

/* --------------------------------------------------------------------------------
Ressemble fort à microshell ou pipex : beaucoup de lignes pour simplement exécuter un fichier avec le bon exécutable dans le bon environnement

Le texte généré par l'exécutable est récupéré et retourné à la fin de la fonction
-------------------------------------------------------------------------------- */
std::string exec_CGI(std::string filePwd, char** envp, Requete& req, Servers* serv)
{
	std::string execPwd = search_exec(filePwd, envp);
	if (execPwd == "")
	{
		std::cerr << "Bad file" << std::endl;
		return ("");
	}

	int fdIn;
	int fd_in[2];
	int fd_out[2];
	char *tab[3];
	char** my_env;
	std::vector<std::string> env;
	pid_t pid = fork();
	char buff[2041] = {0};														// /!\ Comprendre pourquoi 2040
	std::string ret = "";
	int  i;

	tab[0] = (char *)execPwd.c_str();											// Chemin vers l'exécutable qui sera nécessaire
	tab[1] = (char *)filePwd.c_str();											// Chemin vers le fichier à exécuter --> Utiles pour execve
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
			write(fd_in[1], req.get_full_body().c_str(), req.get_len()); // req.get_body ou req.getFullBody
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
			ret += std::string(buff);											// Lire une à une toutes les lignes du résultat du script CGI
		}
		close(fd_out[0]);
		return ret;																// Retourner le résultat final du script en une seule string comprenant toutes les lignes
	}
	return "";
}
