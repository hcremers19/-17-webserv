#include "all_includes.hpp"

/* --- ACCESSORS --- */

int		Client::get_client_socket() const
{
	return this->_clientSocket;
}

int		Client::get_n_server() const
{
	return this->_nServer;
}

/* --- MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Initialiser les informations de base de la classe Client
-------------------------------------------------------------------------------- */
void	Client::init(int i)
{
	lastTime = requestSize = 0;
	this->_nServer = i;
	bzero(request, MAX_REQUEST_SIZE + 1);
}

/* --------------------------------------------------------------------------------
Initialiser la variable _clientSocket avec le retour de accept() et la configu-
rer en mode non-bloquant
-------------------------------------------------------------------------------- */
void	Client::set_socket_client(int sock)
{
	this->_clientSocket = sock;

	fcntl(this->_clientSocket, F_SETFL, O_NONBLOCK);
	// if (setsockopt(this->_clientSocket, SOL_SOCKET, SO_RCVTIMEO, NULL, 0) < 0)
	//     return;
}
