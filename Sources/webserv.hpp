#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <arpa/inet.h>
# include <cstring>
# include <cstdlib>
# include <dirent.h>
# include <fcntl.h>
# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <strings.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <unistd.h>
# include <vector>

# include "HTML/Request.hpp"
# include "Server/Location.hpp"
# include "Server/Server.hpp"
# include "CGI/CGI.hpp"
# include "Config/Config.hpp"
# include "HTML/Client.hpp"
# include "colors.hpp"
# include "Socket/Socket.hpp"
# include "Host/Host.hpp"

class Config;
class Location;
class Server;

# define EXCEPTION public std::exception
# define WHAT const char * what () const

class ArgvErr : EXCEPTION		{WHAT throw () {return ("Usage: ./webserv <config_file>");} };
class DirMissing : EXCEPTION	{WHAT throw () {return ("Error: Missing a directive");} };
class DirTwice : EXCEPTION		{WHAT throw () {return ("Error: Twice the same directive");} };
class DirWrong : EXCEPTION		{WHAT throw () {return ("Error: Directive doesn't exist");} };
class DirWrongPlace : EXCEPTION	{WHAT throw () {return ("Error: Directive is in the wrong place");} };
class ErrorPage : EXCEPTION		{WHAT throw () {return ("Error: Error page is wrong");} };
class IndexLoc : EXCEPTION		{WHAT throw () {return ("Error: Missing index in location");} };
class ListingErr : EXCEPTION	{WHAT throw () {return ("Error: dir_listing must be on or off");} };
class MethWrong : EXCEPTION		{WHAT throw () {return ("Error: Wrong method");} };
class MissingArgv : EXCEPTION	{WHAT throw () {return ("Error: Missing argument after a directive");} };
class NotINT : EXCEPTION		{WHAT throw () {return ("Error: Argument needs to be a number");} };
class RequestErr : EXCEPTION	{WHAT throw () {return ("Error: Wrong request method");} };
class RootErr : EXCEPTION		{WHAT throw () {return ("Error: In root path");} };
class SizeErr : EXCEPTION		{WHAT throw () {return ("Error: Client size");} };
class TooMuchArgv : EXCEPTION	{WHAT throw () {return ("Error: Too many arguments after a directive");} };

#endif /* WEBSERV_HPP */