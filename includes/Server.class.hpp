#ifndef SERVER_CLASS_HPP
# define SERVER_CLASS_HPP

# include "all_includes.hpp"

// Variable globale dans laquelle tout va tourner
class Server
{
	public:
		void				accept_client();
		void				handle_request();
		void				init_server();
		// bool	is_timeout(Client client);										// Undefined
		void				wait_client();
		void				postMethod(Client client, std::string url, Requete req);
		std::vector<Socket> getSocketList() {return this->_sockets; }
		std::vector<Client> getClientsList() {return this->_clients; }

		std::vector<Servers*> servers;

		char **envp;

		int		max_fd;
		fd_set	readSet;
		fd_set	writeSet;

		Location*	loc;
		std::string	query;

	private:
		void		addtowait(int socket, fd_set* set);
		void		selectfd(fd_set* read, fd_set* write);
		void		get_method(Client& client, std::string url, Requete req);
		void		deleteMethod(Client& client, std::string url);
		void		showError(int err, Client& client);
		void		showPage(Client client, std::string dir, int code);
		void		rep_listing(int socket, std::string path, std::string fullurl, Client client);
		void		do_redir(Client client, std::string url);
		bool		kill_client(Client client, Requete req);
		bool		kill_client(Client client);
		bool		is_allowed(std::vector<std::string> methodlist, std::string methodreq);
		bool		is_cgi(std::string filename);
		bool		writewithpoll(std::string url, Client client, Requete req);
		bool		writewithpoll(std::string url, Client client, std::string str);
		std::string	getRootPatch(std::string url, int i);
		Location*	getLocation(std::string url, int i);

		std::vector<Socket> 		_sockets;
		std::vector<Client> 		_clients;
		std::map<int, std::string>	_errors;
};

bool		is_request_done(char *request, size_t header_size, size_t sizereq);
char*		ft_strnstr(const char *haystack, const char *needle, size_t n);
std::string find_type(std::string dir);

#endif