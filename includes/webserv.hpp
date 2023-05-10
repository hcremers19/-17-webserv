#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "all_includes.hpp"

using std::cout;
using std::endl;
using std::cerr;

class Location;
class Servers;
class Conf;

/* cmd f */
/* PARSE_CONF */
void		parsing(int argc, char **argv, Conf &data);
void		parse_basic(int argc, char **argv);
int			count_words(std::string line);
std::string	ft_first_word(std::string line);
std::string	ft_last_word(std::string line);
std::string	ft_last_part(std::string line);
bool		my_atoi(std::string word);

/* Errors */
# define EXCEPTION public std::exception
# define WHAT const char * what () const

class ArgvErr : EXCEPTION		{WHAT throw () { return ("Usage : ./Webserv <config_file>"); }};
class MissingArgv : EXCEPTION	{WHAT throw () { return ("Error: Missing argument after a directive"); }};
class TooMuchArgv : EXCEPTION	{WHAT throw () { return ("Error: Too much arguments after a directive"); }};
class DirWrongPlace : EXCEPTION	{WHAT throw () { return ("Error: Directive is in the wrong place"); }};
class DirWrong : EXCEPTION		{WHAT throw () { return ("Error: Directive doesn't exist"); }};
class DirMissing : EXCEPTION	{WHAT throw () { return ("Error: Missing a directive"); }};
class NotINT : EXCEPTION		{WHAT throw () { return ("Error: Argument needs to be a number"); }};
class MethWrong : EXCEPTION		{WHAT throw () { return ("Error: Method is wrong"); }};
class ErrorPage : EXCEPTION		{WHAT throw () { return ("Error: Error page is wrong"); }};
class DirTwice : EXCEPTION		{WHAT throw () { return ("Error: Two times the same directive"); }};
class RequestErr : EXCEPTION	{WHAT throw () { return ("Error: Request method wrong"); }};
class RootErr : EXCEPTION		{WHAT throw () { return ("Error: In root path"); }};
class IndexLoc : EXCEPTION		{WHAT throw () { return ("Error: Missing index in location"); }};
class ListingErr : EXCEPTION	{WHAT throw () { return ("Error: Dir_listing must be on or off"); }};
class SizeErr : EXCEPTION		{WHAT throw () { return ("Error: Client size"); }};

#endif