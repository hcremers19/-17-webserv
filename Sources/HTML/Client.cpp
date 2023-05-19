#include "../webserv.hpp"

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
Initialize the basic information of the Client class
-------------------------------------------------------------------------------- */
void	Client::init(int i)
{
	lastTime = requestSize = 0;
	this->_nServer = i;
	bzero(request, MAX_REQUEST_SIZE + 1);
}

/* --------------------------------------------------------------------------------
Initialize the _clientSocket variable with the return of accept()
--> a fd, that of the socket associated with this client
Set the socket to non-blocking mode
-------------------------------------------------------------------------------- */
void	Client::set_socket_client(int sock)
{
	this->_clientSocket = sock;

	fcntl(this->_clientSocket, F_SETFL, O_NONBLOCK);
}
