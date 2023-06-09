#ifndef CLIENT_HPP
# define CLIENT_HPP

# define MAX_REQUEST_SIZE 65536

# include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Class that contains the information of the client that sent the HTTP request to
the server
-------------------------------------------------------------------------------- */
class Client
{
	public:
		int		get_client_socket() const;
		int		get_n_server() const;

		void	init(int i);
		void	set_socket_client(int sock);

		int			requestSize;
		char		request[MAX_REQUEST_SIZE + 1];
		size_t		lastTime;
		std::string	finalRequest;

	private:
		int _clientSocket;
		int _nServer;

};

#endif /* CLIENT_HPP */