#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "all_includes.hpp"

class Socket
{
	public:
		void setup(std::string port, std::string ip); // setup the server and client socket
		// void start(); // Wait for client																// Undefined
		// void showPage(std::string dir); // Send a request to the server for show page requested		// Undefined
		// void wait_client(); // Wait for client to connect											// Undifined

		int getServerSocket() { return _serverSocket; }

		timeval time_start;

	private:
		int _serverSocket;

};

#endif