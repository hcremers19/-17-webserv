#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Class used to create and store the socket of a server
-------------------------------------------------------------------------------- */
class Socket
{
	public:
		int		get_server_socket() {return _serverSocket;}
		void	setup(std::string port, std::string ip);

	private:
		int		_serverSocket;

};

#endif /* SOCKET_HPP */