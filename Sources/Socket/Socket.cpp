#include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Setup the base socket of one server (listening socket)

- Initialize the sockaddr_in structure, which contains important information about sockets
- socket()					Create the socket with its domain, type and protocol (for TCP/IP sockets, we use the IP domain, a virtual circuit service, and no additional protocols)
- fcntl()					Set to non-blocking mode
- setsockopt(SO_REUSEADDR)	Allow the program to reuse the same port (allows to restart the program faster/immediately)
- bind()					Bind the fd created with socket() to the address structure and all its options
- listen()					Allow connections to the socket, for a maximum of 42
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
	if ((bind(this->_serverSocket, (struct sockaddr*)&address, sizeof(address))) < 0)
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
		char		buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "[%H:%M:%S]", timeinfo);
		std::cout << colors::yellow << buffer << "[" << port << "] listening..." << colors::reset << std::endl;
	}
}
