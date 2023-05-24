#include "../webserv.hpp"

/* --------------------------------------------------------------------------------
Constructor

Receive the request as a string and parse its different information to initial-
ize the different variables of the class
-------------------------------------------------------------------------------- */
Request::Request(const char* rqst)
{
	this->_requestChar = (char*)rqst;

	std::stringstream ss(rqst);

	this->_request = rqst;
	this->_len = 0;

	ss >> this->_method >> this->_url >> this->_protocol;

	this->make_query();

	if (this->_method == "GET")
		this->parse_GET(ss);
	else if (this->_method == "POST")
		this->parse_POST(ss);
}

/* --------------------------------------------------------------------------------
Destructor
-------------------------------------------------------------------------------- */
Request::~Request()
{
	this->_header.clear();
}


/* --- MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Check that the method is correct (GET, POST or DELETE), otherwise, return error
405
Then check that the protocol is correct, otherwise return error 505
-------------------------------------------------------------------------------- */
int Request::check_method_and_protocol()
{
	return
	(
		(
			this->_method != "POST"
			&& this->_method != "GET"
			&& this->_method != "DELETE") ?
				405 : ((this->_protocol != "HTTP/1.1") ?
					505 : -1
		)
	);
}

/* --------------------------------------------------------------------------------
If the request method is POST, store all relevant information from the HTTP re-
quest header in the _header map and in the private attributes of the class
-------------------------------------------------------------------------------- */
void Request::parse_POST(std::stringstream& ss)
{
	std::string	token;
	std::string	value;
	std::string	key;
	std::string	buff;
	size_t		pos = this->_request.find("\r\n\r\n");

	while (ss >> token)
	{
		// std::cout << colors::bright_magenta << "Token = " << token << colors::reset << std::endl;

		if (token.find("boundary=") != std::string::npos)
			this->_boundary = token.substr(token.find("boundary=") + 9);
		if (token == "Content-Length:")
		{
			if (!key.empty() && !value.empty() && key != token)
			{
				if (ft_back(value) == ' ')
					ft_pop_back(value);
				this->_header.insert(std::pair<std::string, std::string>(key, value));
				value.clear();
			}
			key = token;
			ss >> token;
			this->_header.insert(std::pair<std::string, std::string>(key, token));
			this->_len = atoi(token.c_str());
			key.clear();
		}
		else if (this->_request.find(token) >= pos - token.length())
		{
			pos += 4;
			if (!key.empty() && key != token)
			{
				if (!value.empty())
					ft_pop_back(value);
				this->_header.insert(std::pair<std::string, std::string>(key, (value.empty()) ? token : value));
			}
			size_t posHeader = pos;

			while (pos < this->_len + posHeader)
				this->_bodyFull += this->_requestChar[pos++];
			if (this->_boundary.empty())
				this->_body = this->_bodyFull;
			break;
		}
		else if (ft_back(token) == ':')
		{
			if (!key.empty() && key != token)
			{
				if (ft_back(value) == ' ')
					ft_pop_back(value);
				this->_header.insert(std::pair<std::string, std::string>(key, value));
			}
			ft_pop_back(token);
			key = token;
			value.clear();
		}
		else
			value += ((value.empty()) ? "" : " ") + token;
	}
}

/* --------------------------------------------------------------------------------
If the request method is POST, store all relevant information from the HTTP re-
quest header in the _header map
-------------------------------------------------------------------------------- */
void Request::parse_GET(std::stringstream& ss)
{
	std::string token, value, key;

	while (ss >> token)
	{
		// std::cout << colors::bright_magenta << "Token = " << token << colors::reset << std::endl;

		if (ft_back(token) == ':')
		{
			if (!key.empty() && !value.empty() && key != token)
			{
				if (ft_back(value) == ' ')
					ft_pop_back(value);
				this->_header.insert(std::pair<std::string, std::string>(key, value));
			}
			ft_pop_back(token);
			key = token;
			value.clear();
		}
		else
			value += ((value.empty()) ? "" : " ") + token;
	}
	if (!value.empty() && !key.empty())
	{
		if (ft_back(value) == ' ')
			ft_pop_back(value);
		this->_header.insert(std::pair<std::string, std::string>(key, value));
	}
}

/* --------------------------------------------------------------------------------
Isolate the query
-------------------------------------------------------------------------------- */
void Request::make_query()
{
	size_t pos = this->_url.find("?");
	if (pos != std::string::npos)
		this->_query= this->_url.substr(pos + 1);
}


/* --- NON MEMBER FUNCTIONS --- */

/* --------------------------------------------------------------------------------
Replicate the functioning of the C++11 function std::string.back()
Returns a reference to the last character of the string
-------------------------------------------------------------------------------- */
char&	ft_back(std::string& str)
{
	return (str[str.size() - 1]);
}

/* --------------------------------------------------------------------------------
Replicate the functioning of the C++11 function std::string.pop_back()
Erases the last character of the string, effectively reducing its length by one
-------------------------------------------------------------------------------- */
void	ft_pop_back(std::string& str)
{
	if (!str.empty())
		str.resize(str.size() - 1);
}
