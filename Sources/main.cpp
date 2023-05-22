#include "webserv.hpp"

/* --------------------------------------------------------------------------------
Global variable in which most other variables will run
-------------------------------------------------------------------------------- */
Host host;

/* --------------------------------------------------------------------------------
Main function of the configuration file parsing
First checks if the file passed as argument is valid, then calls all the other
functions used to store the configuration file information in the Config class
-------------------------------------------------------------------------------- */
void parsing(int argc, char** argv, Config& data)
{
	if (argc != 2)
		throw ArgvErr();

	std::ifstream file(argv[1]);
	if (!file)
		throw ArgvErr();

	std::string name = std::string(argv[1]);
	if (name.find(".conf") == std::string::npos)
		throw ArgvErr();

	data.read_file(argv[1]);
	data.init_file_pos();
	data.check_directive();
	data.store_data();
	data.check_data();
}

/* --------------------------------------------------------------------------------
Manage Ctrl+C: close all open sockets (listening or request) and close the pro-
gram cleanly
-------------------------------------------------------------------------------- */
void sig_handler(int signal)
{
	(void)signal;

	for (size_t i = 0; i < host.get_socket_list().size(); i++)
		close(host.get_socket_list()[i].get_server_socket());
	for (size_t i = 0; i < host.get_client_list().size(); i++)
		close(host.get_client_list()[i].get_client_socket());

	std::cout << colors::grey << colors::on_red << "\nServer killed, bye!" << colors::reset << std::endl;

	exit(0);
}

/* --------------------------------------------------------------------------------
Main function in which webserv runs
Calls the parsing of the configuration file, initializes the host, retrieves the
signals and launches the main loop where the interactions with the server are
processed

data: Configuration structure in which a lot of data are stored temporarily, be-
fore being exported to the global variable
-------------------------------------------------------------------------------- */
int main(int ac, char** av, char** envp)
{
	Config data;

	try
	{
		parsing(ac, av, data);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}

	host.init_host(envp, &data);
	signal(SIGINT, sig_handler);

	while (19)
	{
		host.wait_client();
		host.accept_client();
		host.handle_request();
	}
}
