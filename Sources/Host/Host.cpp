#include "../webserv.hpp"

/* --- MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
La fonction "add_to_wait" prend en entrée un socket et un pointeur vers un en-
semble de sockets (fd_set). Elle ajoute le socket à l'ensemble en utilisant la
fonction FD_SET, puis met à jour la valeur maximale de socket (maxFd) de l'ob-
jet Host si le socket donné en entrée est plus grand que la valeur actuelle de
maxFd.

Source : ChatGPT

Se renseigner mieux sur FD_SET
-------------------------------------------------------------------------------- */
void Host::add_to_wait(int socket, fd_set* set)
{
	FD_SET(socket, set);
	if (socket > this->maxFd)
		this->maxFd = socket;
}

/* --------------------------------------------------------------------------------
La fonction "select_fd" prend en entrée deux pointeurs vers des ensembles de so-
ckets (fd_set), un pour les sockets à lire (read) et l'autre pour les sockets à
écrire (write). Elle appelle la fonction "select" pour surveiller les ensembles
de sockets donnés en entrée, en attendant que l'un d'entre eux soit prêt à être
lu ou écrit. Si select() renvoie une valeur inférieure à 0, cela indique une er-
reur, dans ce cas, le programme se termine avec un code de sortie -1. Si
select() renvoie 0, cela signifie que la fonction a expiré avant qu'un socket
soit prêt, et un message "select() time out" est affiché. Sinon, les ensembles
de sockets à écrire et à lire sont mis à jour avec les sockets correspondants
dans l'objet Host.

Source : ChatGPT
-------------------------------------------------------------------------------- */
void Host::select_fd(fd_set* read, fd_set* write)
{
	int r = 0;
	if ((r = select(this->maxFd + 1, read, write, 0, 0)) < 0)
		exit(-1);
	else if (r == 0)
		std::cout << "select() time out" << std::endl;
	this->writeSet = *write;
	this->readSet = *read;
}

/* --------------------------------------------------------------------------------
La fonction "wait_client" initialise les ensembles de sockets read et write à
zéro (utilisant la fonction FD_ZERO). Elle ajoute ensuite les sockets du serveur
et les sockets des clients à l'ensemble read en appelant la fonction
"add_to_wait". Enfin, elle appelle la fonction "select_fd" avec les ensembles de
sockets read et write. Cela permet d'attendre qu'un ou plusieurs sockets soient
prêts à être lus ou écrits, en surveillant tous les sockets présents dans l'en-
semble read.

Source : ChatGPT

--> En gros, ici je pense qu'on parle des sockets de base, ceux qui écoutent, il
y en a donc un seul par serveur configuré
Se renseigner mieux sur FD_ZERO et fd_set
-------------------------------------------------------------------------------- */
void Host::wait_client()
{
	fd_set read;
	fd_set write;

	FD_ZERO(&read);
	FD_ZERO(&write);
	for (size_t i = 0; i < this->_sockets.size(); i++) // Set fd of server
		this->add_to_wait(this->_sockets[i].get_server_socket(), &read);
	for (size_t i = 0; i < this->_clients.size(); i++) // Set fd of server
		this->add_to_wait(this->_clients[i].get_client_socket(), &read);
	this->select_fd(&read, &write);
}

/* --------------------------------------------------------------------------------
Accepter le client, configurer sa classe, l'enregistrer dans _clients
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
			std::cout << colors::green << "New connection!" << colors::reset << std::endl; // Tim
		}
	}
}

/* --------------------------------------------------------------------------------
Fonction principale de gestion des requêtes reçues
- Reçoit la requête
- Sépare le header et le body
- Traite les informations du header
- Traite le message en fonction de la méthode demandée
- Exécute le script CGI
-------------------------------------------------------------------------------- */
void Host::handle_request()
{
	for (size_t i = 0; i < this->_clients.size(); i++)							// Faire cette action pour tous les clients différents
	{
		if (FD_ISSET(this->_clients[i].get_client_socket(), &this->readSet))		// Vérifier que le fd existe bien
		{
			size_t Reqsize = recv(this->_clients[i].get_client_socket(), this->_clients[i].request, MAX_REQUEST_SIZE, 0); // Recevoir le message de requête et stocker sa taille dans une variable
			this->_clients[i].requestSize += Reqsize;

			for (size_t size = 0; size < Reqsize; size++)
				this->_clients[i].finalRequest.push_back(this->_clients[i].request[size]);

			int header_size = this->_clients[i].finalRequest.find("\r\n\r\n", 0); // Calculer la taille du header de la requête pour pouvoir l'isoler
			header_size += 4;

			if (Reqsize == 0)													// Kill le client si la requête est de taille nulle
			{
				std::cout << colors::on_bright_red << "Connection is closed!" << colors::reset << std::endl;
				this->kill_client(this->_clients[i]);
				i--;
			}
			else if (is_request_done((char *)this->_clients[i].finalRequest.c_str(), header_size, this->_clients[i].requestSize)) // Vérifier qu'on a bien toute la requête
			{
				std::cout << colors::bright_cyan << "== New request! ==" << colors::reset << std::endl;
				Request requete((char*)this->_clients[i].finalRequest.c_str()); // Construire la classe requête sur base de la string reçue
				int ret = -1;
				if ((ret = requete.check_method()) != -1)							// Vérifier le nom de la méthode
				{
					this->show_error_page(ret, this->_clients[i]);
					if (this->kill_client(this->_clients[i]))
						i--;
					continue;
				}
				std::string urlrcv = requete.get_url();
				size_t pos;
				if ((pos = urlrcv.rfind("?")) != std::string::npos)				// Trouver un "?" dans l'URL   (???)
				{
					this->query = urlrcv.substr(pos, urlrcv.size()); 
					urlrcv = urlrcv.substr(0, pos);
				}
				std::string tmpstr;
				int			tmpint; 
				std::istringstream(this->servers[this->_clients[i].get_n_server()]->get_body_size()) >> tmpint;
				if (requete.get_len() != std::string::npos && requete.get_len() > (size_t)tmpint) // Vérifier que la requête n'est pas trop longue
				{
					this->show_error_page(413, this->_clients[i]);
					if (this->kill_client(this->_clients[i]))
						i--;
					continue;
				}

				this->loc = this->get_location(urlrcv, this->_clients[i].get_n_server()); // Obtenir l'emplacement du fichier pointé par l'url

				if (!((this->loc == NULL) ? this->is_allowed(this->servers[this->_clients[i].get_n_server()]->get_method(),
					requete.get_method()) : this->is_allowed(this->loc->get_method(), requete.get_method()))
						&& urlrcv.find("cgi_bin") == std::string::npos)			// Vérifier si la méthode est autorisée ou pas
				{
					std::cout << "Unautorised method " << requete.get_method() << "!" << std::endl;
					show_error_page(405, this->_clients[i]);
					if (this->kill_client(this->_clients[i]))
						i--;
					continue;
				}
				if (this->is_cgi(urlrcv))										// Vérifier si le fichier à traiter est pris en charge par notre serveur
				{
					std::cout << colors::blue << "CGI start!" << colors::reset << std::endl;

					std::string urlsend = this->get_root_path(urlrcv, this->_clients[i].get_n_server()); // URL à donner au script CGI
					std::string rescgi = exec_CGI(urlsend, this->envp, requete, this->servers[this->_clients[i].get_n_server()]); // Exécuter le script CGI
					if (rescgi.empty())
						this->show_error_page(404, this->_clients[i]);

					rescgi = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + rescgi; // Raccourcissable avec =+
					int r = send(this->_clients[i].get_client_socket(), rescgi.c_str(), rescgi.size(), 0); // Envoyer le résultat final sur le socket afin qu'il soit affiché
					if (r < 0)
						this->show_error_page(500, this->_clients[i]);
					else if (r == 0)
						this->show_error_page(400, this->_clients[i]);
				}
				else
				{
					if (this->loc && !this->loc->get_redir().empty())
						this->do_redir(this->_clients[i], this->loc->get_redir()); // Redirections, pas sûr de comprendre
					else if (requete.get_method() == "GET")
						this->GET_method(this->_clients[i], urlrcv);
					else if (requete.get_method() == "POST")
						this->POST_method(this->_clients[i], urlrcv, requete);
					else if (requete.get_method() == "DELETE")
						this->DELETE_method(this->_clients[i], urlrcv);
				}
				if (this->kill_client(this->_clients[i]))				// Effacer ce que le serveur sait du client et fermer son socket puisqu'il ne lui sera plus nécessaire
					i--;
				if (i <= 0)
				{
					this->_clients[i].requestSize = 0;
					bzero(this->_clients[i].request, MAX_REQUEST_SIZE);
				}
			}
		}
	}
	usleep(10000);																// Attendre pendant 10 ms (pourquoi ?)
}

/* --------------------------------------------------------------------------------
Exécuter la méthode GET

Supprimer la variable req si elle est inutile
NE PAS CONFONDRE avec le getter de la variable _method dans les classes Loca-
tion, Request et Server

(Besoin de plus d'explications)
Se renseigner sur le listing
-------------------------------------------------------------------------------- */
void Host::GET_method(Client& client, std::string urlrcv)
{
	std::cout << colors::bright_yellow << "GET method!" << colors::reset << std::endl;

	if (urlrcv.size() >= 64)													// L'URL ne peut pas être plus long que 64 caractères (en général, ou seulement ici ?)
	{
		this->show_error_page(414, client);
		return;
	}
	struct stat path_stat;														// Structure stat qui permet de récolter et de stocker des informations sur un fichier, pour les réutiliser plus tard

	std::string urlsend = this->get_root_path(urlrcv, client.get_n_server());

	if (this->loc && !(this->loc->get_index().empty()) && (strcmp(urlrcv.c_str(), \
		this->loc->get_dir().c_str()) == 0))
	{
		this->show_page(client, this->loc->get_root() + this->loc->get_index(), 200);	// AFficher la page si tout est ok
		return;
	}

	FILE* fd = fopen(urlsend.c_str(), "rb");
	stat(urlsend.c_str(), &path_stat);

	if (fd == NULL)
	{
		std::cout << colors::on_bright_blue << "Resource not found: "<< urlsend << colors::reset << std::endl;
		this->show_error_page(404, client);											// Erreur 404, la page n'a pas été trouvée
	}
	else
	{
		// Si l'url reçu pointe vers un dossier, et que le listing est activé, afficher la page de listing des fichiers
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
Exécuter la méthode DELETE

Besoin de plus d'explications
-------------------------------------------------------------------------------- */
void Host::DELETE_method(Client& client, std::string urlrcv)
{
	std::cout << colors::bright_yellow << "DELETE method!" << colors::reset << std::endl;
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
Exécuter la méthode POST

Besoin de plus d'explications
-------------------------------------------------------------------------------- */
void Host::POST_method(Client client, std::string url, Request req)
{
	if (req.get_header()["Transfer-Encoding"] == "chunked")
	{
		this->show_error_page(411, client);
		return;
	}
	std::string urlsend = this->get_root_path(url, client.get_n_server());
	struct stat buf;
	lstat(urlsend.c_str(), &buf);

	if (S_ISDIR(buf.st_mode))
	{
		std::string name;
		size_t start = 0;
		size_t end = 0;
		std::string body = req.get_full_body();
		std::string file;
		if (!(req.get_header()["Content-Type"].empty()) && !(req.get_boundary().empty()))
		{
			std::cout << colors::on_cyan <<  "Post in directory: " << colors::reset << colors::green << std::endl;
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
		std::cout << "Post in file" << std::endl;
		if (!this->write_with_poll(urlsend, client, req))
			return;
	}
	if (req.get_len() == 0)
		this->show_page(client, "", 204);
	else
		this->show_page(client, "", 201);
}

/* --------------------------------------------------------------------------------
Initialiser le socket de base pour chacun des serveurs que l'on veut faire tour-
ner dans le programme
Insérer les codes d'erreur dans _errors

socket : socket de base, d'écoute, peut être instancié plusieurs fois si la con-
figuration de webserv demande de configurer plusieurs serveurs
-------------------------------------------------------------------------------- */
void Host::init_server()
{
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
	this->_errors.insert(std::make_pair(408, "408 Request Timeout"));
	this->_errors.insert(std::make_pair(411, "411 Length Required"));
	this->_errors.insert(std::make_pair(413, "413 Request Entity Too Large"));
	this->_errors.insert(std::make_pair(414, "414 Request-URI Too Long"));
	this->_errors.insert(std::make_pair(500, "500 Internal Server Error"));
	this->_errors.insert(std::make_pair(502, "502 Bad Gateway"));
	this->_errors.insert(std::make_pair(505, "505 HTTP Version Not Supported"));
	this->maxFd = -1;
}

/* --------------------------------------------------------------------------------
Envoyer sur le socket le message et la page d'erreur correspondant à l'int reçu
en premier paramètre
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
			std::string msg = "HTTP/1.1 " + it->second + "\nContent-Type: text/plain\nContent-Length: " + ft_to_string<int>(it->second.size()) + "\n\n" + it->second + "\n";
			int sendret = send(client.get_client_socket(), msg.c_str(), msg.size(), 0);
			if (sendret < 0)
				std::cout << "Client disconnected" << std::endl;
			else if (sendret == 0)
				std::cout << "0 byte passed to server" << std::endl;
		}
	}
}

/* --------------------------------------------------------------------------------
Fermer le socket attribué au client passé en paramètre et puis l'effacer de la
liste de clients enregistrés
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
Vérifie si la méthode reçue par le client fait partie des méthodes autorisées ou
pas
-------------------------------------------------------------------------------- */
bool Host::is_allowed(std::vector<std::string> methodlist, std::string methodreq)
{
	for (size_t i = 0; i < methodlist.size(); i++)
		if (methodlist[i] == methodreq)
			return true;
	return false;
}

/* --------------------------------------------------------------------------------
Renvoie l'url à donner au script CGI ou où aller chercher la page html à afficher
-------------------------------------------------------------------------------- */
std::string Host::get_root_path(std::string urlrcv, int i)
{
	std::string urlroot = this->servers[i]->get_root();
	if (urlroot[urlroot.size() - 1] == '/')
		urlroot.erase(urlroot.size() - 1, 1);
	if (this->loc && !(this->loc->get_root().empty()))
		urlrcv.erase(urlrcv.find(this->loc->get_dir()), urlrcv.find(this->loc->get_dir()) + this->loc->get_dir().size());

	std::cout << "Url To Send: " << colors::green << urlroot + urlrcv << colors::reset << std::endl;
	return urlroot + urlrcv;
}

/* --------------------------------------------------------------------------------
Vérifier si le fichier à traiter est pris en charge par notre serveur (python ou
perl)
-------------------------------------------------------------------------------- */
bool Host::is_cgi(std::string filename)
{
	std::vector<std::string>  cgi_list;
	cgi_list.push_back(".py");
	cgi_list.push_back(".pl");
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
Envoyer sur le socket la page à afficher
(Peut-être besoin de plus d'explications)

Espaces superflus à supprimer dans cette fonction
-------------------------------------------------------------------------------- */
void Host::show_page(Client client, std::string dir, int code)
{
	int r;

	if (dir != "")
		std::cout << colors::on_cyan << "Show page: " << dir << colors::reset << std::endl;

	if (dir.empty())
	{
		std::string err_msg = "HTTP/1.1 " + this->_errors[code] + "\n\n";
		if ((r = send(client.get_client_socket(), err_msg.c_str(), err_msg.size(), 0)) < 0)
			this->show_error_page(500, client);
		else if (r == 0)
			this->show_error_page(400, client);
		return;
	}
	else
	{
		FILE* fd_s = fopen(dir.c_str(), "rb");									// Ouvrir le fichier passé en paramètre
		fseek (fd_s, 0, SEEK_END);												// Positionner le curseur à la fin du fichier
		int lSize = ftell (fd_s);												// Enregistrer le nombre de caractères que fait le fichier
		rewind (fd_s);															// Revenir au début du fichier (pourquoi, si on le ferme juste après ?)
		fclose(fd_s);

		std::string type = find_type(dir);

		std::string msg = "HTTP/1.1 " + this->_errors.find(code)->second + "\n" + "Content-Type: " + type + "\nContent-Length: " + ft_to_string<int>(lSize) + "\n\n"; // Concaténer toutes les infos du header de réponse HTTP dans une string
		int ret = send(client.get_client_socket(), msg.c_str(), msg.size(), 0);	// Envoyer la string générée sur le socket
		if (ret < 0)
			this->show_error_page(500, client);
		else if (ret == 0)
			this->show_error_page(400, client);

		int fd_r = open(dir.c_str(), O_RDONLY);									// ... Je comprends plus grand chose à partir d'ici jusqu'à la fin du fichier
		if (fd_r < 0)
		{
			this->show_error_page(500, client);
			return;
		}

		// ADD to read queue =================================
		this->add_to_wait(fd_r, &this->readSet);
		this->select_fd(&this->readSet, &this->writeSet);

		// ==========================

		char file[1024];
		int r2;
		int r = read(fd_r, file, 1024);
		if (r < 0)
			this->show_error_page(500, client);
		else // Get big file
		{
			while (r)
			{
				if ((r2 = send(client.get_client_socket(), file, r, 0)) < 0)
				{
					this->show_error_page(500, client);
					break;
				}
				else if (r2 == 0)
				{
					this->show_error_page(400, client);
					break;
				}

				// ADD to read queue =================================
				this->add_to_wait(fd_r, &this->readSet);
				this->select_fd(&this->readSet, &this->writeSet);
				// ==========================


				if ((r = read(fd_r, file, 1024)) < 0)
				{
					this->show_error_page(500, client);
					break;
				}
				if (r == 0)
					break;
			}
		}
		close(fd_r);
	}
}

/* --------------------------------------------------------------------------------
Si l'url reçu pointe vers un dossier, et que le listing est activé, afficher la page de listing des fichiers

Pas ouf comme façon de faire parce que la page à afficher est littéralement codée en html ici et ne permet aucune modification de style

Les liens n'apparaissent pas dans le bon ordre mais je ne sais pas si c'est à cause de readdir qui le fait mal, ou s'il y a une erreur dans le code
-------------------------------------------------------------------------------- */
void Host::directory_listing(int socket, std::string path, std::string fullurl, Client client)
{
	std::cout << colors::on_cyan << "Show repository listing" << colors::reset << std::endl;

	DIR* dir;
	struct dirent* entry;
	std::string data;

	std::string response = "HTTP/1.1 200 OK\n\n<!DOCTYPE html>\n<html>\n<body>\n<h1>" + path + "</h1>\n<pre>\n";

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
	response += "</pre>\n</body>\n</html>\n";
	int r = send(socket, response.c_str(), response.size(), 0);
	if (r < 0)
		this->show_error_page(500, client);
	else if (r == 0)
		this->show_error_page(400, client);										// Bad request
}

/* --------------------------------------------------------------------------------
Écrire dans l'url passé en paramètre le contenu de la requête POST
Le fd du fichier dans lequel on veut écrire est d'abord mis en file d'attente pour s'assurer qu'il n'y ait aucun chevauchement

Cette fonction possède une surcharge dans laquelle on passe toute la classe Request pour aller y chercher la string voulue
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
		this->show_error_page(500, client);										// Internal server error
		close(fd);
		return false;
	}
	close(fd);
	return true;
}

/* --------------------------------------------------------------------------------
Écrire dans l'url passé en paramètre le contenu de la requête POST
Le fd du fichier dans lequel on veut écrire est d'abord mis en file d'attente pour s'assurer qu'il n'y ait aucun chevauchement

Cette fonction possède une surcharge dans laquelle on ne passe que la string nécessaire et non pas toute la classe Request
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
Obtenir l'emplacement du fichier pointé par l'url
Passe par le vecteur "_locations" de la classe Server pour y chercher le fichier demandé
-------------------------------------------------------------------------------- */
Location* Host::get_location(std::string url, int i)
{
	std::vector<Location*> locs = this->servers[i]->get_location();
	for (size_t i = 0; i < locs.size(); i++)
	{
		if (strncmp(locs[i]->get_dir().c_str(), url.c_str(), locs[i]->get_dir().size()) == 0)
		{
			std::cout << colors::on_cyan << url << " is the location!" << colors::reset << std::endl;
			return locs[i];
		}
	}
	return NULL;
}

/*
En cas de redirection (lien qui pointe vers un autre site)

Envoie un message de réponse HTML pour rediriger le client vers le site désiré
*/
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
Trouver l'extension du fichier reçu en paramètre pour inclure le bon type de contenu dans le message de réponse http

Sûrement une meilleure façon de faire avec std::string que de repasser par un char* pour utiliser strcmp -_-

Pas vraiment de fonction C++ qui permet de trouver la dernière occurence d'une std::string
find_last_of() le fait avec un caractère mais ne retourne que sa position sous forme de size_t
-------------------------------------------------------------------------------- */
std::string find_type(std::string dir)
{
	char *dot = strrchr((char *)dir.c_str(), '.');
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
Fonction de la libft qui reproduit le comportement de la fonction strnstr, qui n'est pas disponible sur toutes les plateformes

Elle recherche la chaîne de caractères "needle" dans la chaîne de caractères "haystack" jusqu'au carctère en position "n", et retourne un pointeur vers le premier caractère de la chaîne trouvée, ou NULL le cas échéant
-------------------------------------------------------------------------------- */
char*	ft_strnstr(const char *haystack, const char *needle, size_t n)
{
	size_t	in;																	// Index de needle
	size_t	ih;																	// Index de haystack
	char	*hs;																// Copie de haystack

	ih = 0;
	hs = (char *)haystack;
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
Fonction en C qui détermine si la requête a bien terminé d'être envoyée par le
client/reçue par le serveur
-------------------------------------------------------------------------------- */
bool is_request_done(char *request, size_t header_size, size_t sizereq)
{
	size_t sizebody = sizereq - header_size;

	char *body = strstr(request, "\r\n\r\n");
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
		char *start = ft_strnstr(request, "Content-Length: ", sizereq - sizebody) + 16;
		char *end = strstr(start, "\r\n");
		char *len = strndup(start, end - start);
		int len_i = atoi(len);
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
Converts a numeric value to std::string.
-------------------------------------------------------------------------------- */
template<typename T>
std::string ft_to_string(const T& x)
{
	std::ostringstream oss;

	oss << x;
	return (oss.str());
}
