#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <arpa/inet.h>
# include <cstring>
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

class Location;
class Server;
class Config;

# define EXCEPTION public std::exception
# define WHAT const char * what () const

class ArgvErr : EXCEPTION		{WHAT throw () {return ("Usage : ./Webserv <config_file>");} };
class MissingArgv : EXCEPTION	{WHAT throw () {return ("Error: Missing argument after a directive");} };
class TooMuchArgv : EXCEPTION	{WHAT throw () {return ("Error: Too much arguments after a directive");} };
class DirWrongPlace : EXCEPTION	{WHAT throw () {return ("Error: Directive is in the wrong place");} };
class DirWrong : EXCEPTION		{WHAT throw () {return ("Error: Directive doesn't exist");} };
class DirMissing : EXCEPTION	{WHAT throw () {return ("Error: Missing a directive");} };
class NotINT : EXCEPTION		{WHAT throw () {return ("Error: Argument needs to be a number");} };
class MethWrong : EXCEPTION		{WHAT throw () {return ("Error: Method is wrong");} };
class ErrorPage : EXCEPTION		{WHAT throw () {return ("Error: Error page is wrong");} };
class DirTwice : EXCEPTION		{WHAT throw () {return ("Error: Two times the same directive");} };
class RequestErr : EXCEPTION	{WHAT throw () {return ("Error: Request method wrong");} };
class RootErr : EXCEPTION		{WHAT throw () {return ("Error: In root path");} };
class IndexLoc : EXCEPTION		{WHAT throw () {return ("Error: Missing index in location");} };
class ListingErr : EXCEPTION	{WHAT throw () {return ("Error: Dir_listing must be on or off");} };
class SizeErr : EXCEPTION		{WHAT throw () {return ("Error: Client size");} };

#endif /* WEBSERV_HPP */