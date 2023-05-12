#include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Setup le socket de base du serveur, avec le port et l'ip reçus dans le fichier
de configuration, le laisser en attente d'une nouvelle requête

Rajouter plus d'explications sur les différentes fonctions standard des sockets
et la structure "address"
-------------------------------------------------------------------------------- */
void Socket::setup(std::string port, std::string ip)
{
	int port_int;
	if (!(std::istringstream(port) >> port_int))
	{
		perror("istringstream conversion failed: ");
		exit(-1);
	}
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port_int);

	if (ip.empty())
		ip = "0.0.0.0";

	if ((this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("serverSocket: ");
		exit(-1);
	}

	fcntl(this->_serverSocket, F_SETFL, O_NONBLOCK);

	int value = 1;
	if (setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) == -1)
	{
		perror("setsockopt:");
		exit(-1);
	}

	address.sin_addr.s_addr = inet_addr(ip.c_str());
	if ((bind(this->_serverSocket, (struct sockaddr *)&address, sizeof(address))) < 0)
	{
		perror("bind");
		exit(-1);
	}
	if (listen(this->_serverSocket, 42) != 0)
	{
		perror("Listening");
		exit(-1);
	}
	else
	{
		time_t		rawtime;
		struct tm*	timeinfo;
		char buffer	[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "[%H:%M:%S]", timeinfo);
		std::cout << colors::yellow << buffer << "[" << port << "] listening ..." << colors::reset << std::endl;
	}
}
