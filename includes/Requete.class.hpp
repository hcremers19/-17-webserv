#ifndef REQUETE_CLASS_HPP
# define REQUETE_CLASS_HPP

# include "all_includes.hpp"


class Requete
{
	public:
		Requete(char *requete);
		~Requete();

		size_t								get_len() const			{return this->_len;};
		std::map<std::string, std::string>	get_header() const		{return this->_header;};
		std::string							get_body() const		{return this->_body;};
		std::string							get_boundary() const	{return this->_boundary;};
		std::string							get_full_body() const	{return this->_fullBody;};
		std::string							get_method() const		{return this->_method;};
		std::string							get_protocol() const	{return this->_protocol;};
		std::string							get_query() const		{return this->_query;};
		std::string							get_request() const		{return this->_request;};
		std::string							get_url() const			{return this->_url;};

		int		check_method();

	protected:
		void	make_POST(std::stringstream& ss);
		void	make_GET(std::stringstream& ss);
		void	make_query();

		char*								_charRequest;	// FULL Request
		size_t								_len;			// Len defined in header
		std::map<std::string, std::string>	_header;		// Header
		std::string							_body;			// Content body without boundary
		std::string							_boundary;		// Boundary defined in header
		std::string							_fullBody;		// Content body with boundary
		std::string							_method;		// Method defined in header
		std::string							_protocol;		// Protocol defined in header
		std::string							_query;			// Query text after url without "?"
		std::string							_request;		// FULL Request
		std::string							_url;			// URL from Header
};

#endif