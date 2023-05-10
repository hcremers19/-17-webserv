#include "all_includes.hpp"

/* --------------------------------------------------------------------------------
Variable globale dans laquelle vont tourner la plupart des autres variables
-------------------------------------------------------------------------------- */
Server serv;

void deleteallfd(Server serv)
{
	for (size_t i = 0; i < serv.getSocketList().size(); i++)
		close(serv.getSocketList()[i].getServerSocket());
	for (size_t i = 0; i < serv.getClientsList().size(); i++)
		close(serv.getClientsList()[i].get_client_socket());
	std::cout << colors::green << "Clean all Fd" << colors::reset << std::endl;
}

/* --------------------------------------------------------------------------------
Gérer Ctrl+C : fermer tous les sockets ouverts (d'écoute ou de requête) et fer-
mer proprement le programme
-------------------------------------------------------------------------------- */
void sig_handler(int signal)
{
	(void)signal;

	for (size_t i = 0; i < serv.getSocketList().size(); i++)
		close(serv.getSocketList()[i].getServerSocket());
	for (size_t i = 0; i < serv.getClientsList().size(); i++)
		close(serv.getClientsList()[i].get_client_socket());

	std::cout << colors::grey << colors::on_red << "\nServer killed, bye!" << colors::reset << std::endl;

	exit(0);
}

/* --------------------------------------------------------------------------------
Fonction principale dans laquelle tourne webserv

data : Structure de configuration dans laquelle sont stockées plein de données
temporairement, avant d'être exportées vers la variable globale

--> Devrait pouvoir être supprimée pour être directement intégrée à serv
-------------------------------------------------------------------------------- */
int main(int ac, char **av, char **envp)
{
	Conf data;

	try
	{
		parsing(ac, av, data);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}

	serv.servers = data.get_servers();
	serv.envp = envp; 															// Inclure l'environnement dans serv, pourrait peut-être être déplacé dans une autre fonction
	serv.init_server();
	signal(SIGINT, sig_handler);

	while (1)																	// Boucle principale dans laquelle sera traitée chaque interaction avec le serveur
	{
		serv.wait_client();														// Séparer les fonctions relatives à ces 3 lignes dans des fichiers différents ?
		serv.accept_client();
		serv.handle_request();
	}
}
