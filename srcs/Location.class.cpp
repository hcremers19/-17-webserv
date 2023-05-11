#include "webserv.hpp"

/* --------------------------------------------------------------------------------
Constructor
-------------------------------------------------------------------------------- */
Location::Location()
{
	return;
}

/* --------------------------------------------------------------------------------
Destructor
-------------------------------------------------------------------------------- */
Location::~Location()
{
	_method.clear();
}

/* --- ACCESSORS --- */

void	Location::set_dir(std::string word)		
{
    this->_dir = word;
}

void	Location::set_root(std::string word)		
{
    this->_root = word;
}

void	Location::set_index(std::string word)		
{
    this->_index = word;
}

void	Location::set_method(std::string word)	
{
    this->_method.push_back(word);
}

void	Location::set_listing(std::string word)	
{
    this->_listing = word;
}

void	Location::set_redir(std::string word)		
{
    this->_redir = word;
}


std::string					Location::get_dir()		
{
    return this->_dir;
}

std::vector<std::string>	Location::get_method()		
{
    return this->_method;
}

std::string					Location::get_root()		
{
    return this->_root;
}

std::string					Location::get_index()		
{
    return this->_index;
}

std::string					Location::get_listing()	
{
    return this->_listing;
}

std::string					Location::get_redir()		
{
    return this->_redir;
}
