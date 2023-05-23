#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Class that contains all the information of the request received by the server
-------------------------------------------------------------------------------- */
class Request
{
	public:
		Request(const char* rqst);
		~Request();

		size_t								get_len() const			{return this->_len;};
		std::map<std::string, std::string>	get_header() const		{return this->_header;};
		std::string							get_body() const		{return this->_body;};
		std::string							get_boundary() const	{return this->_boundary;};
		std::string							get_full_body() const	{return this->_bodyFull;};
		std::string							get_method() const		{return this->_method;};
		std::string							get_protocol() const	{return this->_protocol;};
		std::string							get_query() const		{return this->_query;};
		std::string							get_request() const		{return this->_request;};
		std::string							get_url() const			{return this->_url;};

		int		check_method_and_protocol();

	protected:
		void	parse_POST(std::stringstream& ss);
		void	parse_GET(std::stringstream& ss);
		void	make_query();

		std::map<std::string, std::string>	_header;		// Full header as a map
		size_t								_len;			// Length defined in header
		std::string							_boundary;		// Boundary defined in header
		std::string							_method;		// Method defined in header
		std::string							_protocol;		// Protocol defined in header
		std::string							_body;			// Content body without boundary
		std::string							_bodyFull;		// Content body with boundary
		std::string							_query;			// Query text after url without "?"
		char*								_requestChar;	// FULL Request as a char*
		std::string							_request;		// FULL Request as a string
		std::string							_url;			// URL from Header
};

char&	ft_back(std::string& str);
void	ft_pop_back(std::string& str);

#endif /* REQUEST_HPP */