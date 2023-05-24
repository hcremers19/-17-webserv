#include "../webserv.hpp"

/* --- MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Use the FD_SET() function to add to the set of fd's the socket passed in parame-
ter, and then update the fd max so that it is not smaller than the fd value of
the socket in question
-------------------------------------------------------------------------------- */
void Host::add_to_wait(int socket, fd_set* set)
{
	FD_SET(socket, set);
	if (socket > this->maxFd)
		this->maxFd = socket;
}

/* --------------------------------------------------------------------------------
Get the sets of read and write sockets
Use select() to monitor them until one of them is ready to be read or written
Update the read and write fd_set member attributes if no errors have occurred
-------------------------------------------------------------------------------- */
void Host::select_fd(fd_set* read, fd_set* write)
{
	int r = 0;
	if ((r = select(this->maxFd + 1, read, write, 0, 0)) < 0)
		exit(EXIT_FAILURE);
	else if (r == 0)
		std::cout << "select() time out" << std::endl;
	this->writeSet = *write;
	this->readSet = *read;
}

/* --------------------------------------------------------------------------------
Initialize the fd sets fd_set which gather the fds of the listening sockets of
each server of the program, and which will allow to manage these fds with
select() to determine if they are ready to be used
We then send them to add_to_wait() and select_fd() to put them already on hold
-------------------------------------------------------------------------------- */
void Host::wait_client()
{
	fd_set read;
	fd_set write;

	FD_ZERO(&read);
	FD_ZERO(&write);
	for (size_t i = 0; i < this->_sockets.size(); i++)
		this->add_to_wait(this->_sockets[i].get_server_socket(), &read);
	for (size_t i = 0; i < this->_clients.size(); i++)
		this->add_to_wait(this->_clients[i].get_client_socket(), &read);
	this->select_fd(&read, &write);
}

/* --------------------------------------------------------------------------------
Accept the client, configure its class, register it in _clients

--> Register the fd of the socket assigned to this client
-------------------------------------------------------------------------------- */
void Host::accept_client()
{
	sockaddr_in addrclient;
	socklen_t clientSize = sizeof(addrclient);

	for (size_t i = 0; i < this->_sockets.size(); i++)
	{
		if (FD_ISSET(this->_sockets[i].get_server_socket(), &this->readSet))
		{
			Client client;
			client.init(i);
			client.set_socket_client(accept(this->_sockets[i].get_server_socket(), (struct sockaddr*)&addrclient, &clientSize));
			this->_clients.push_back(client);
			if (client.get_client_socket() < 0)
			{
				perror("Connect");
				exit(-1);
			}
			std::cout << colors::green << "New connection!" << colors::reset << std::endl;
		}
	}
}

/* --------------------------------------------------------------------------------
Main function of management of received requests

For each active client:
- FD_ISSET(): Check that the read fd has been set
- Receive the request with recv() and store its size
- Calculate the size of the header to be able to isolate it from the body, then kill the client if the size of the request is null
- Once we are sure to have the whole request (is_request_done()):
	- Build the Request class with the necessary information
	- Check the method called: check_method()
	- Process the URL by isolating the query if there is one
	- Check if the size of the request is not too big
	- Get the location of the file pointed to by the URL
	- Check if the method is authorized or not
	- Check if the file to process is supported by our server
	- Execute the CGI script by sending it the URL modified according to the path to root
	- Send the result of the CGI to the socket with send() so that it is displayed
	- Process the redirects with do_redir()
	- Delete what the server knows about the client and close its socket since it will no longer be needed
-------------------------------------------------------------------------------- */
void Host::handle_request()
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (FD_ISSET(this->_clients[i].get_client_socket(), &this->readSet))
		{
			size_t Reqsize = recv(this->_clients[i].get_client_socket(), this->_clients[i].request, MAX_REQUEST_SIZE, 0);
			this->_clients[i].requestSize += Reqsize;

			for (size_t size = 0; size < Reqsize; size++)
				this->_clients[i].finalRequest.push_back(this->_clients[i].request[size]);

			int header_size = this->_clients[i].finalRequest.find("\r\n\r\n", 0);
			header_size += 4;

			if (Reqsize < 0)
			{
				std::cout << "Recv failed!" << std::endl;
				show_error_page(500, _clients[i]);
				kill_client(_clients[i]);
				i--;
			}
			if (Reqsize == 0)
			{
				std::cout << colors::on_bright_red << "Connection is closed!" << colors::reset << std::endl;
				this->kill_client(this->_clients[i]);
				i--;
			}
			else if (is_request_done((char*)this->_clients[i].finalRequest.c_str(), header_size, this->_clients[i].requestSize))
			{
				std::cout << colors::bright_cyan << "== New request! ==" << colors::reset << std::endl;

				// std::cout << colors::cyan << "Final request:" << this->_clients[i].finalRequest << colors::reset << std::endl;

				Request	rqst(this->_clients[i].finalRequest.c_str());
				int		ret = -1;
				if ((ret = rqst.check_method_and_protocol()) != -1)
				{
					this->show_error_page(ret, this->_clients[i]);
					if (this->kill_client(this->_clients[i]))
						i--;
					continue;
				}
				std::string	urlrcv = rqst.get_url();
				size_t		pos;
				if ((pos = urlrcv.rfind("?")) != std::string::npos)
				{
					this->query = urlrcv.substr(pos, urlrcv.size());
					urlrcv = urlrcv.substr(0, pos);
				}

				std::string tmpstr;
				int			tmpint;
				std::istringstream(this->servers[this->_clients[i].get_n_server()]->get_body_size()) >> tmpint;
				if (rqst.get_len() != std::string::npos && rqst.get_len() > (size_t)tmpint)
				{
					this->show_error_page(413, this->_clients[i]);
					if (this->kill_client(this->_clients[i]))
						i--;
					continue;
				}

				this->loc = this->get_location(urlrcv, this->_clients[i].get_n_server());

				if (!((this->loc == NULL) ?
					this->is_allowed(this->servers[this->_clients[i].get_n_server()]->get_method(), rqst.get_method()) :
					this->is_allowed(this->loc->get_method(), rqst.get_method())) && urlrcv.find("cgi_bin") == std::string::npos)
				{
					std::cout << "Unauthorised method " << rqst.get_method() << "!" << std::endl;
					show_error_page(405, this->_clients[i]);
					if (this->kill_client(this->_clients[i]))
						i--;
					continue;
				}
				if (this->is_cgi(urlrcv))
				{
					std::cout << colors::blue << "CGI start!" << colors::reset << std::endl;

					std::cout << "handle_request URL = " << urlrcv << std::endl;
					std::string urlsend = this->get_root_path(urlrcv, this->_clients[i].get_n_server());
					std::string rescgi = exec_CGI(urlsend, this->envp, rqst, this->servers[this->_clients[i].get_n_server()]);

					std::cout << colors::bright_magenta << "rescgi: " << colors::reset << rescgi << std::endl;
					if (rescgi.empty())
						this->show_error_page(500, this->_clients[i]);

					rescgi = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + rescgi;
					int r = send(this->_clients[i].get_client_socket(), rescgi.c_str(), rescgi.size(), 0);
					if (r < 0)
						this->show_error_page(500, this->_clients[i]);
					else if (r == 0)
						this->show_error_page(400, this->_clients[i]);
				}
				else
				{
					if (this->loc && !this->loc->get_redir().empty())
						this->do_redir(this->_clients[i], this->loc->get_redir());
					else if (rqst.get_method() == "GET")
						this->GET_method(this->_clients[i], urlrcv);
					else if (rqst.get_method() == "POST")
						this->POST_method(this->_clients[i], urlrcv, rqst);
					else if (rqst.get_method() == "DELETE")
						this->DELETE_method(this->_clients[i], urlrcv);
				}
				if (this->kill_client(this->_clients[i]))
					i--;
				if (i <= 0)
				{
					this->_clients[i].requestSize = 0;
					bzero(this->_clients[i].request, MAX_REQUEST_SIZE);
				}
			}
		}
	}
	usleep(10000);
}

/* --------------------------------------------------------------------------------
Execute the GET method
Retrieve the HTML page requested by the client and send it to the socket to dis-
play it if it was found, otherwise display the error page corresponding to the
error encountered
If the URL points to a folder and dir_listing is activated, display the index of
the files in the folder
-------------------------------------------------------------------------------- */
void Host::GET_method(Client& client, std::string urlrcv)
{
	std::cout << colors::bright_yellow << "GET method!" << colors::reset << std::endl;

	if (urlrcv.size() >= 64)
	{
		this->show_error_page(414, client);
		return;
	}
	struct stat path_stat;

	std::cout << "GET_method URL = " << urlrcv << std::endl;
	std::string urlsend = this->get_root_path(urlrcv, client.get_n_server());

	if (this->loc && !(this->loc->get_index().empty()) && (strcmp(urlrcv.c_str(), \
		this->loc->get_dir().c_str()) == 0))
	{
		this->show_page(client, this->loc->get_root() + this->loc->get_index(), 200);
		return;
	}

	FILE* fd = fopen(urlsend.c_str(), "rb");
	stat(urlsend.c_str(), &path_stat);

	if (fd == NULL)
	{
		std::cout << colors::on_bright_blue << "Resource not found: "<< urlsend << colors::reset << std::endl;
		this->show_error_page(404, client);
	}
	else
	{

		if (S_ISDIR(path_stat.st_mode))
		{
			std::cout << colors::on_bright_blue << "File is a directory!" << colors::reset << std::endl;

			if (strcmp(urlrcv.c_str(), "/") == 0)
				this->show_page(client, urlsend + this->servers[client.get_n_server()]->get_index(), 200);
			else if (this->servers[client.get_n_server()]->get_listing() == "on" || (this->loc && this->loc->get_listing() == "on"))
				this->directory_listing(client.get_client_socket(), urlrcv, urlsend, client);
			else
				this->show_error_page(404, client);
		}
		else
			this->show_page(client, urlsend, 200);
		fclose(fd);
	}
}

/* --------------------------------------------------------------------------------
Execute the DELETE method

Delete the resource indicated by the URL
-------------------------------------------------------------------------------- */
void Host::DELETE_method(Client& client, std::string urlrcv)
{
	std::cout << colors::bright_yellow << "DELETE method!" << colors::reset << std::endl;
	std::cout << "DELETE_method URL = " << urlrcv << std::endl;
	std::string urlsend = this->get_root_path(urlrcv, client.get_n_server());

	FILE* fd = fopen(urlsend.c_str(), "r");
	if (!fd)
	{
		this->show_error_page(404, client);
		return;
	}
	fclose(fd);
	std::remove(urlsend.c_str());

	std::string tosend = "HTTP/1.1 200 OK\n";
	int ret = send(client.get_client_socket(), tosend.c_str(), tosend.size(), 0);
	if (ret < 0)
		this->show_error_page(500, client);
	else if (ret == 0)
		this->show_error_page(400, client);
	std::cout << colors::green << urlsend << " has been deleted!" << colors::reset << std::endl;
}

/* --------------------------------------------------------------------------------
Execute the POST method
Determine whether we need to POST to a file or to a folder, then prepare the
right parameters to send them to the write_with_poll() function
The lstat() function allows to get information about the path passed in parame-
ter, then S_ISDIR() tells if it is a folder or not
-------------------------------------------------------------------------------- */
void Host::POST_method(Client client, std::string url, Request req)
{
	std::cout << colors::bright_yellow << "POST method!" << colors::reset << std::endl;
	if (req.get_header()["Transfer-Encoding"] == "chunked")
	{
		this->show_error_page(411, client);
		return;
	}
	std::cout << "POST_method URL = " << url << std::endl;
	std::string	urlsend = this->get_root_path(url, client.get_n_server());
	struct stat	buf;
	lstat(urlsend.c_str(), &buf);

	if (S_ISDIR(buf.st_mode))
	{
		std::string	name;
		size_t		start = 0;
		size_t		end = 0;
		std::string	body = req.get_full_body();
		std::string	file;
		if (!(req.get_header()["Content-Type"].empty()) && !(req.get_boundary().empty()))
		{
			std::cout << colors::on_cyan << "Post in directory: " << colors::reset << std::endl;
			for (int i = 0; true; i++)
			{
				if ((start = body.find("name=\"", start)) == std::string::npos)
					break;
				start += 6;
				if ((end = body.find("\"", start)) == std::string::npos)
					break;
				name = body.substr(start, end - start);
				std::cout << "+ " + name << std::endl;

				if ((start = body.find("\r\n\r\n", end)) == std::string::npos)
					break;
				start += 4;
				if ((end = body.find(req.get_boundary(), start)) == std::string::npos)
					break;

				file = body.substr(start, end - start - 4);

				if (!this->write_with_poll(urlsend + "/" + name, client, file))
					break;

				if (body[end + req.get_boundary().size()] == '-')
					break;
			}
		}
		else
		{
			this->show_error_page(400, client);
			return;
		}
	}
	else
	{
		std::cout << colors::on_cyan << "Post in file" << colors::reset << std::endl;
		if (!this->write_with_poll(urlsend, client, req))
			return;
	}
	if (req.get_len() == 0)
		this->show_page(client, "", 204);
	else
		this->show_page(client, "", 201);
}

/* --------------------------------------------------------------------------------
Include the configured servers and the environment in the Host class
Initialize the base socket for each server you want to run in the program
Insert the error codes in _errors

socket: basic listening socket, can be instantiated several times if the webserv
configuration requires to configure several servers
-------------------------------------------------------------------------------- */
void Host::init_host(char** env, Config* data)
{
	this->servers = data->get_servers();
	this->envp = env;

	for (size_t i = 0; i < this->servers.size(); i++)
	{
		Socket socket;
		socket.setup(this->servers[i]->get_listen(), this->servers[i]->get_name());
		this->_sockets.push_back(socket);
	}
	this->_errors.insert(std::make_pair(200, "200 OK"));
	this->_errors.insert(std::make_pair(201, "201 Created"));
	this->_errors.insert(std::make_pair(204, "204 No Content"));
	this->_errors.insert(std::make_pair(300, "300 Multiple Choices"));
	this->_errors.insert(std::make_pair(301, "301 Moved Permanently"));
	this->_errors.insert(std::make_pair(302, "302 Found"));
	this->_errors.insert(std::make_pair(303, "303 See Other"));
	this->_errors.insert(std::make_pair(307, "307 Temporary Redirect"));
	this->_errors.insert(std::make_pair(400, "400 Bad Request"));
	this->_errors.insert(std::make_pair(404, "404 Not Found"));
	this->_errors.insert(std::make_pair(405, "405 Method Not Allowed"));
	this->_errors.insert(std::make_pair(411, "411 Length Required"));
	this->_errors.insert(std::make_pair(413, "413 Request Entity Too Large"));
	this->_errors.insert(std::make_pair(414, "414 Request-URI Too Long"));
	this->_errors.insert(std::make_pair(500, "500 Internal Server Error"));
	this->_errors.insert(std::make_pair(505, "505 HTTP Version Not Supported"));
	this->maxFd = -1;
}

/* --------------------------------------------------------------------------------
Send on the socket the message and the error page corresponding to the int re-
ceived in first parameter
-------------------------------------------------------------------------------- */
void Host::show_error_page(int err, Client& client)
{
	std::map<std::string, std::string> errpages = this->servers[client.get_n_server()]->get_error();

	if (errpages.find(ft_to_string<int>(err)) != errpages.end())
	{
		int fd = open(errpages[ft_to_string<int>(err)].c_str(), O_RDONLY);
		if (fd < 0)
		{
			std::cout << colors::on_bright_red << "Show error: " << this->_errors[err] << "!" << colors::reset << std::endl;
			std::cout << colors::on_bright_red << "Pre-config error page doesn't exist: " << errpages[ft_to_string<int>(err)] << colors::reset << std::endl;
			close(fd);
			return;
		}
		close(fd);
		this->show_page(client, errpages[ft_to_string<int>(err)], 200);
	}
	else
	{
		std::map<int, std::string>::iterator it = this->_errors.find(err);
		if (it != this->_errors.end())
		{
			std::cout << colors::on_bright_red << "Show error: " << it->second << "!" << colors::reset << std::endl;
			int fd = open(errpages[ft_to_string<int>(err)].c_str(), O_RDONLY);
			if (fd < 0)
			{
				std::cout << colors::on_bright_red << "Show error: " << this->_errors[err] << "!" << colors::reset << std::endl;
				std::cout << colors::on_bright_red << "Pre-config error page doesn't exist: " << errpages[ft_to_string<int>(err)] << colors::reset << std::endl;
				close(fd);
				return;
			}
			close(fd);
			// this->show_page(client, errpages[ft_to_string<int>(err)], err);
			std::cout << colors::on_bright_red << "Show error : " << it->second << " !" << colors::on_grey << std::endl;
			std::string msg = "HTTP/1.1 " + it->second + "\nContent-Type: text/plain\nContent-Length: " + std::to_string(it->second.size()) + "\n\n" + it->second + "\n";
			int sendret = send(client.get_client_socket() , msg.c_str(), msg.size(), 0);
			if (sendret < 0)
				std::cout << "Client disconnected" << std::endl;
			else if (sendret == 0)
				std::cout << "0 byte passed to server" << std::endl;
		}
	}
}

/* --------------------------------------------------------------------------------
Close the socket assigned to the client passed in parameter and then delete it
from the list of registered clients
-------------------------------------------------------------------------------- */
bool Host::kill_client(Client client)
{
	std::cout << colors::red << "Client killed" << colors::reset << std::endl;
	close(client.get_client_socket());
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].get_client_socket() == client.get_client_socket())
		{
			this->_clients.erase(this->_clients.begin() + i);
			return true;
		}
	}
	exit(1);
}

/* --------------------------------------------------------------------------------
Check if the method received by the client is one of the authorized methods or
not
-------------------------------------------------------------------------------- */
bool Host::is_allowed(std::vector<std::string> methodlist, std::string methodreq)
{
	for (size_t i = 0; i < methodlist.size(); i++)
		if (methodlist[i] == methodreq)
			return true;
	return false;
}

/* --------------------------------------------------------------------------------
Return the URL to give to the CGI script, which will start from the root speci-
fied in the configuration file
-------------------------------------------------------------------------------- */
std::string Host::get_root_path(std::string urlrcv, int i)
{
	std::string urlroot = this->servers[i]->get_root();
	if (urlroot[urlroot.size() - 1] == '/')
		urlroot.erase(urlroot.size() - 1, 1);
	// if (this->loc && !(this->loc->get_root().empty()))
	// 	urlrcv.erase(urlrcv.find(this->loc->get_dir()), urlrcv.find(this->loc->get_dir()) + this->loc->get_dir().size());

	std::cout << "Url To Send: " << colors::green << urlroot + urlrcv << colors::reset << std::endl;
	std::cout << "urlroot: " << colors::green << urlroot << colors::reset << std::endl;
	std::cout << "urlrcv: " << colors::green << urlrcv << colors::reset << std::endl;
	return urlroot + urlrcv;
}

/* --------------------------------------------------------------------------------
Check if the file to process is supported by our server (Python, Perl or PHP)
-------------------------------------------------------------------------------- */
bool Host::is_cgi(std::string filename)
{
	std::vector<std::string> cgi_list;
	cgi_list.push_back(".py");
	cgi_list.push_back(".pl");
	cgi_list.push_back(".php");
	if (filename.find('.') == std::string::npos)
		return false;
	std::string extension = filename.substr(filename.find('.'), filename.size());
	for (size_t i = 0; i < cgi_list.size(); i++)
	{
		if (cgi_list[i] == extension)
			return true;
	}
	return false;
}

/* --------------------------------------------------------------------------------
Send the page to be displayed on the socket

Operation in several steps: send first the header and then the body of the HTTP
response message

First we have to find the type and size of the file to send all the necessary
information in the header
Then once the header is sent, we send the body separately with a loop similar to
get_next_line() which reads and sends line by line the content of the file
-------------------------------------------------------------------------------- */
void Host::show_page(Client client, std::string dir, int code)
{
	int r;

	if (dir != "")
		std::cout << colors::on_cyan << "Show page: " << colors::reset << colors::cyan << dir << colors::reset << std::endl;

	if (dir.empty())
	{
		std::string errMsg = "HTTP/1.1 " + this->_errors[code] + "\n\n";
		if ((r = send(client.get_client_socket(), errMsg.c_str(), errMsg.size(), 0)) < 0)
			this->show_error_page(500, client);
		else if (r == 0)
			this->show_error_page(400, client);
		return;
	}
	else
	{
		int			size = find_size(dir);
		std::string	type = find_type(dir);

		std::string hdr = "HTTP/1.1 " + this->_errors.find(code)->second + "\n" + "Content-Type: " + type + "\nContent-Length: " + ft_to_string<int>(size) + "\n\n";
		int ret = send(client.get_client_socket(), hdr.c_str(), hdr.size(), 0);
		if (ret < 0)
			this->show_error_page(500, client);
		else if (ret == 0)
			this->show_error_page(400, client);

		int fd_r = open(dir.c_str(), O_RDONLY);
		if (fd_r < 0)
		{
			this->show_error_page(500, client);
			return;
		}

		this->add_to_wait(fd_r, &this->readSet);
		this->select_fd(&this->readSet, &this->writeSet);

		char bdy[1024];
		int rd2;
		int rd = read(fd_r, bdy, 1024);
		if (rd < 0)
			this->show_error_page(500, client);
		else
		{
			while (rd)
			{
				if ((rd2 = send(client.get_client_socket(), bdy, rd, 0)) < 0)
				{
					this->show_error_page(500, client);
					break;
				}
				else if (rd2 == 0)
				{
					this->show_error_page(400, client);
					break;
				}

				this->add_to_wait(fd_r, &this->readSet);
				this->select_fd(&this->readSet, &this->writeSet);

				if ((rd = read(fd_r, bdy, 1024)) < 0)
				{
					this->show_error_page(500, client);
					break;
				}
				if (rd == 0)
					break;
			}
		}
		close(fd_r);
	}
}

/* --------------------------------------------------------------------------------
If the URL received points to a folder, and the listing is enabled, generate a
directory listing page and display it directly
-------------------------------------------------------------------------------- */
void Host::directory_listing(int socket, std::string path, std::string fullurl, Client client)
{
	std::cout << colors::on_cyan << "Show repository listing" << colors::reset << std::endl;

	DIR* dir;
	struct dirent* entry;
	std::string data;

	std::string response = "HTTP/1.1 200 OK\n\n<!DOCTYPE html>\n<html lang=\"en\">\n\
<head>\n<title>Index of " + path + "</title>\
<meta charset=\"utf-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
<link rel=\"stylesheet\" href=\"style.css\">\n\
<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.0-beta1/dist/css/bootstrap.min.css\" rel=\"stylesheet\"\
	integrity=\"sha384-0evHe/X+R7YkIZDRvuzKMRqM+OrBnVFBL6DOitfPri4tjfHxaWutUpFmBp4vmVor\" crossorigin=\"anonymous\">\n\
<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,700\" rel=\"stylesheet\">\n\
<style>\nbody {\ntext-align: center rigth;\nbackground-color: #b0c6db;\nbackground-image: url('img/land.jpg');\n\
background-repeat: no-repeat;\nbackground-size: cover;\nbackground-position: center center;\nfont-family: 'Open Sans', \
sans-serif;\nfont-weight: 300;\nmargin-left: 50px;\n}\n</style>\n</head>\n\
<body>\n<br><h2>Index of " + path + "</h2><br>\n<pre>\n";

	if ((dir = opendir(fullurl.c_str())) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
			response += "<a href=\"" + ((std::string(entry->d_name) == ".") ? std::string(path) : (std::string(path) + "/" + std::string(entry->d_name))) + "\">" + std::string(entry->d_name) + "</a>\n";
		closedir(dir);
	}
	else
	{
		perror("Directory listing");
		return;
	}

	response += "\t\t</pre>\n\t</body>\n</html>\n";
	int r = send(socket, response.c_str(), response.size(), 0);
	if (r < 0)
		this->show_error_page(500, client);
	else if (r == 0)
		this->show_error_page(400, client);
}

/* --------------------------------------------------------------------------------
Write in the URL passed in parameter the content of the POST request, using
write() and the fd of the file to write in
The fd of the socket to write to is first queued to make sure there is no over-
lap

This function has an overload in which the whole Request class is passed to
fetch the desired string
This overload is called in the case of POSTing to a directory
-------------------------------------------------------------------------------- */
bool Host::write_with_poll(std::string url, Client client, std::string str)
{
	int r = 0;
	int fd = open(url.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (fd < 0)
	{
		this->show_error_page(500, client);
		close(fd);
		return false;
	}

	this->add_to_wait(fd, &this->writeSet);
	this->select_fd(&this->readSet, &this->writeSet);

	r = write(fd, str.c_str(), str.size());
	if (r < 0)
	{
		this->show_error_page(500, client);
		close(fd);
		return false;
	}
	close(fd);
	return true;
}

/* --------------------------------------------------------------------------------
Write in the URL passed in parameter the content of the POST request, using
write() and the fd of the file to write in
The fd of the socket to write to is first queued to make sure there is no over-
lap

This function has an overload in which only the necessary string is passed and
not the whole Request class
This overload is called in the case of POSTing to a file
-------------------------------------------------------------------------------- */
bool Host::write_with_poll(std::string url, Client client, Request req)
{
	int r = 0;
	int fd = open(url.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (fd < 0)
	{
		this->show_error_page(500, client);
		close(fd);
		return false;
	}

	this->add_to_wait(fd, &this->writeSet);
	this->select_fd(&this->readSet, &this->writeSet);

	std::cout << colors::green << req.get_full_body() << colors::reset << std::endl;
	r = write(fd, req.get_full_body().c_str(), req.get_full_body().size());
	if (r < 0)
	{
		this->show_error_page(500, client);
		close(fd);
		return false;
	}
	close(fd);
	return true;
}

/* --------------------------------------------------------------------------------
Get the location of the file pointed by the URL
Go through the "_locations" vector of the Server class to look for the requested
file
-------------------------------------------------------------------------------- */
Location* Host::get_location(std::string url, int i)
{
	std::vector<Location*> locs = this->servers[i]->get_location();
	for (size_t i = 0; i < locs.size(); i++)
	{
		if (strncmp(locs[i]->get_dir().c_str(), url.c_str(), locs[i]->get_dir().size()) == 0)
		{
			std::cout << colors::on_cyan << url << " is a location" << colors::reset << std::endl;
			return locs[i];
		}
	}
	return NULL;
}

/* --------------------------------------------------------------------------------
In case of redirection (link that goes to another site)

Generate and send an HTML response message to redirect the client to the desired
site
-------------------------------------------------------------------------------- */
void Host::do_redir(Client client, std::string url)
{
	std::cout << colors::on_cyan << "Is a redirection to: " << url << colors::reset << std::endl;
	std::string tosend = "HTTP/1.1 200 OK\n\n<head><meta http-equiv = \"refresh\" content = \"0; url =" + url + "\" /></head>";

	int r = send(client.get_client_socket(), tosend.c_str(), tosend.size(), 0);
	if (r < 0)
		this->show_error_page(500, client);
	else if (r == 0)
		this->show_error_page(400, client);
}


/* --- NON-MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Find the size of the file received in parameter to include it in the HTTP re-
sponse message
-------------------------------------------------------------------------------- */
int			find_size(std::string dir)
{
	FILE* fd_s = fopen(dir.c_str(), "rb");
	fseek(fd_s, 0, SEEK_END);
	int lSize = ftell(fd_s);
	rewind(fd_s);
	fclose(fd_s);

	return lSize;
}

/* --------------------------------------------------------------------------------
Find the extension of the file received as a parameter to include the right con-
tent type in the http response message

Not really a C++ function that finds the last occurrence of a std::string better
than this
find_last_of() does it with a single character but only returns its position as
a size_t
-------------------------------------------------------------------------------- */
std::string find_type(std::string dir)
{
	char*	dot = strrchr((char*)dir.c_str(), '.');
	if (strcmp(dot, ".css") == 0) return "text/css";
	if (strcmp(dot, ".jpeg") == 0) return "image/jpeg";
	if (strcmp(dot, ".jpg") == 0) return "image/jpg";
	if (strcmp(dot, ".gif") == 0) return "image/gif";
	if (strcmp(dot, ".png") == 0) return "image/png";
	if (strcmp(dot, ".js") == 0) return "application/javascript";
	if (strcmp(dot, ".mp4") == 0) return "video/mp4 ";
	if (strcmp(dot, ".json") == 0) return "application/json";
	if (strcmp(dot, ".pdf") == 0) return "application/pdf";
	if (strcmp(dot, ".html") == 0) return "text/html";

	return "text/plain";
}

/* --------------------------------------------------------------------------------
libft function that reproduces the behavior of the strnstr function, which is
not available on all platforms

It searches for the string "needle" in the string "haystack" up to the character
in position "n", and returns a pointer to the first character of the string
found, or NULL if not found
-------------------------------------------------------------------------------- */
char*	ft_strnstr(const char* haystack, const char* needle, size_t n)
{
	size_t	in;
	size_t	ih;
	char*	hs;

	ih = 0;
	hs = (char*)haystack;
	if (!strlen(needle))
		return (hs);
	if ((strlen(haystack) < strlen(needle)) || n < strlen(needle))
		return (0);
	while (hs[ih] && ih <= n - strlen(needle))
	{
		in = 0;
		while (needle[in] && (hs[ih + in] == needle[in]))
			in++;
		if (in == strlen(needle))
			return (&hs[ih]);
		ih++;
	}
	return (0);
}

/* --------------------------------------------------------------------------------
C function that determines if the request has finished being sent by the client/
received by the server, taking into account if the request is chunked or not, or
if a boundary has been defined or not
-------------------------------------------------------------------------------- */
bool is_request_done(const char* request, size_t header_size, size_t sizereq)
{
	size_t	sizebody = sizereq - header_size;

	char*	body = strstr((char*)request, "\r\n\r\n");
	if (!body)
		return false;
	body += 4;
	if (ft_strnstr(request, "chunked", sizereq - sizebody))
	{
		if (strstr(body, "\r\n\r\n"))
			return true;
		return false;
	}
	else if (ft_strnstr(request, "Content-Length", sizereq - sizebody))
	{
		char*	start = ft_strnstr(request, "Content-Length: ", sizereq - sizebody) + 16;
		char*	end = strstr(start, "\r\n");
		char*	len = strndup(start, end - start);
		int		len_i = atoi(len);

		free(len);
		if ((size_t)len_i <= sizebody)
			return true;
		return false;
	}
	else if (ft_strnstr(request, "boundary=", sizereq - sizebody))
	{
		if (strstr(body, "\r\n\r\n"))
			return true;
		return false;
	}
	return true;
}

/* --------------------------------------------------------------------------------
Converts a numeric value to std::string
-------------------------------------------------------------------------------- */
template<typename T>
std::string ft_to_string(const T& x)
{
	std::ostringstream oss;

	oss << x;
	return (oss.str());
}
