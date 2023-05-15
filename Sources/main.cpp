#include "webserv.hpp"

/* --------------------------------------------------------------------------------
Variable globale dans laquelle vont tourner la plupart des autres variables
-------------------------------------------------------------------------------- */
Host host;

/* --------------------------------------------------------------------------------
Fonction principale du parsing du fichier de configuration
Vérifie d'abord si le fichier passé en argument est valide, puis appelle toutes les autres fonctions servant à stocker les informations du fichier de configuration dans la classe Config
-------------------------------------------------------------------------------- */
void parsing(int argc, char **argv, Config& data)
{
	if (argc != 2)
		throw ArgvErr();

	std::ifstream file(argv[1]);
	if (!file)
		throw ArgvErr();

	std::string name = std::string(argv[1]);
	if (name.find(".conf") == std::string::npos) // npos == -1 in size_t
		throw ArgvErr();

	data.read_file(argv[1]);
	data.init_file_pos();
	data.check_directive();
	data.stock_data();
	data.check_data();
}

/* --------------------------------------------------------------------------------
Gérer Ctrl+C : fermer tous les sockets ouverts (d'écoute ou de requête) et fer-
mer proprement le programme
-------------------------------------------------------------------------------- */
void sig_handler(int signal)
{
	(void)signal;

	for (size_t i = 0; i < host.get_socket_list().size(); i++)
		close(host.get_socket_list()[i].get_server_socket());
	for (size_t i = 0; i < host.get_client_list().size(); i++)
		close(host.get_client_list()[i].get_client_socket());

	std::cout << colors::grey << colors::on_red << "\nServer killed, bye!" << colors::reset << std::endl;

	exit(0);
}

/* --------------------------------------------------------------------------------
Fonction principale dans laquelle tourne webserv

data : Structure de configuration dans laquelle sont stockées plein de données
temporairement, avant d'être exportées vers la variable globale

--> Devrait pouvoir être supprimée pour être directement intégrée à host
-------------------------------------------------------------------------------- */
int main(int ac, char **av, char **envp)
{
	Config data;

	try
	{
		parsing(ac, av, data);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}

	host.servers = data.get_servers();
	host.envp = envp; 															// Inclure l'environnement dans host, pourrait peut-être être déplacé dans une autre fonction
	host.init_server();
	signal(SIGINT, sig_handler);

	while (19)																	// Boucle principale dans laquelle sera traitée chaque interaction avec le serveur
	{
		host.wait_client();														// Séparer les fonctions relatives à ces 3 lignes dans des fichiers différents ?
		host.accept_client();
		host.handle_request();
	}
}