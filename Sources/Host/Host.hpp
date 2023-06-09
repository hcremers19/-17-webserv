#ifndef HOST_HPP
# define HOST_HPP

# include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Global variable in which everything will run
-------------------------------------------------------------------------------- */
class Host
{
	public:
		void				accept_client();
		void				handle_request();
		void				wait_client();
		void				init_host(char** env, Config* data);
		std::vector<Socket> get_socket_list() {return this->_sockets;}
		std::vector<Client> get_client_list() {return this->_clients;}

		std::vector<Server*>	servers;

		char**	envp;

		int		maxFd;
		fd_set	readSet;
		fd_set	writeSet;

		Location*	loc;
		std::string	query;

	private:
		void		add_to_wait(int socket, fd_set* set);
		void		select_fd(fd_set* read, fd_set* write);
		void		GET_method(Client& client, std::string url);
		void		DELETE_method(Client& client, std::string url);
		void		show_error_page(int err, Client& client);
		void		show_page(Client client, std::string dir, int code);
		void		directory_listing(int socket, std::string path, std::string fullurl, Client client);
		void		do_redir(Client client, std::string url);
		bool		kill_client(Client client);
		bool		is_allowed(std::vector<std::string> methodlist, std::string methodreq);
		bool		is_cgi(std::string filename);
		void		POST_method(Client client, std::string url, Request req);
		bool		write_with_poll(std::string url, Client client, Request req);
		bool		write_with_poll(std::string url, Client client, std::string str);
		std::string	get_root_path(std::string url, int i);
		Location*	get_location(std::string url, int i);

		std::vector<Socket> 		_sockets;
		std::vector<Client> 		_clients;
		std::map<int, std::string>	_errors;
};

bool		is_request_done(const char* request, size_t header_size, size_t sizereq);
char*		ft_strnstr(const char* haystack, const char* needle, size_t n);
int			find_size(std::string dir);
std::string find_type(std::string dir);

template<typename T>
std::string ft_to_string(const T& x);

#endif /* HOST_HPP */