/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   RequestParser.cpp								  :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: pdelanno <pdelanno@student.42.fr>		  +#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2023/10/24 12:52:22 by pdelanno		  #+#	#+#			 */
/*   Updated: 2023/10/31 10:23:48 by pdelanno		 ###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

# include "../includes/RequestParser.hpp"


/**************************************************************/
/*				 DEFAULT CON-/DESTRUCTOR					*/
/**************************************************************/

RequestParser::RequestParser()
{
	_method = "";
	_path = "";
	_query = "";
	_body = "";
}

RequestParser::RequestParser( std::string buffer )
{
	_method = "";
	_path = "";
	_query = "";
	_body = "";
	parseRequest(buffer);
}

RequestParser::~RequestParser() {}


/**************************************************************/
/*				 GETTER & SETTER METHODS					*/
/**************************************************************/

std::string RequestParser::getMethod()
{
	return(_method);
}
std::string RequestParser::getPath()
{
	return(_path);
}
std::string RequestParser::getQuery()
{
	return(_query);
}
std::string RequestParser::getProtocol()
{
	return(_protocol);
}
std::map<std::string, std::string> RequestParser::getHeaders()
{
	return(_headers);
}
std::string RequestParser::getBody()
{
	return(_body);
}


/**************************************************************/
/*					  PUBLIC METHODS						*/
/**************************************************************/

std::string RequestParser::removeCarriageReturn(std::string &str)
{
	std::string::iterator it = str.begin();
	while (it != str.end())
	{
		if (*it == '\r')
			it = str.erase(it);
		else
			++it;
	}
	return (str);
}

void RequestParser::parseRequest(std::string const &buffer)
{
	//std::cout << "----------Buffer: " << std::endl << buffer << std::endl;
	//splitBuffer()
	std::stringstream linestream(buffer);
	std::getline(linestream, _method, ' ');
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		throw std::runtime_error("Error: invalid request type");
	std::getline(linestream, _path, ' ');
	if (_path[0] != '/')
		throw std::runtime_error("Error: invalid path in request");
	if (_path.find("?") != std::string::npos)
	{
		std::stringstream sstream(_path);
		std::getline(sstream, _path, '?');
		std::getline(sstream, _query);
	}
	std::getline(linestream, _protocol, '\n');
	std::string key;
	std::string value;
	while (linestream)
	{
		std::getline(linestream, key, ':');
		if (key[0] == '\r' && (_method == "POST" || _method == "DELETE"))
		{
			key.erase(0, 2);
			_body = key;
			break ;
		}
		if (key.empty() || key[0] == '\r')
			break ;
		//removeCarriageReturn(key);
		std::getline(linestream, value);
		if (value.empty())
			break ;
		value = value.erase(0, 1);
		_headers[key] = value;
	}
	/* std::cout << "---------------------------------All headers: " << std::endl;
	std::map<std::string, std::string>::iterator it;
	it = _headers.begin();
	for (it = _headers.begin(); it != _headers.end(); ++it)
		std::cout << it->first << " " << it->second << std::endl; */
}

void RequestParser::parseHeaders()
{
	    
}


/**************************************************************/
/*					OVERLOAD OPERATOR					   */
/**************************************************************/

std::ostream &operator<<(std::ostream &str, RequestParser &rp)
{
	str << "Method: " << rp.getMethod() << std::endl;
	str << "Path: " << rp.getPath() << std::endl;
	str << "Query: " << rp.getQuery() << std::endl;
	str << "Protocol: " << rp.getProtocol() << std::endl;
	str << "Body: " << rp.getBody() <<std::endl;
	//str << "Headers: ";
	// std::map<std::string, std::string>::iterator it;
	// for (it = rp.getHeaders().begin(); it != rp.getHeaders().end(); ++it)
	//	 str << it->first << " " << it->second << std::endl;
	return (str);
};
