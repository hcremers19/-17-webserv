#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "../webserv.hpp"

class Socket
{
	public:
		int		get_server_socket() {return _serverSocket;}

		void	setup(std::string port, std::string ip); // setup the server and client socket
		// void start(); // Wait for client																// Undefined
		// void show_page(std::string dir); // Send a request to the server for show page requested		// Undefined
		// void wait_client(); // Wait for client to connect											// Undifined

	private:
		int _serverSocket;

};

#endif