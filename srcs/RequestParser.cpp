/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 12:52:22 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/08 09:28:13 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/RequestParser.hpp"


/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

RequestParser::RequestParser()
{
    _method = NO_TYPE;
    _type = NONE;
    _path = "";
    _query = "";
    _host = "";
    _contentLength = 0;
    _body = "";
    _bodyLength = 0;
}

RequestParser::RequestParser(std::string buffer, ssize_t contentLength)
{
	_method = NO_TYPE;
    _type = NONE;
	_path = "";
	_query = "";
    _host = "";
    _contentLength = contentLength;
	_body = "";
    _bodyLength = 0;
	parseRequest(buffer);
}

RequestParser::~RequestParser() {}


/**************************************************************/
/*                 GETTER & SETTER METHODS                    */
/**************************************************************/

httpMethod RequestParser::getMethod() const
{
    return(_method);
}
std::string RequestParser::getPath() const
{
    return(_path);
}
std::string RequestParser::getQuery() const
{
    return(_query);
}
std::string RequestParser::getProtocol() const
{
    return(_protocol);
}
std::map<std::string, std::string> RequestParser::getHeaders() const
{
    return(_headers);
}
std::string RequestParser::getBody() const
{
    return(_body);
}
ssize_t RequestParser::getContentLength() const
{
    return(_contentLength);
}
std::string RequestParser::getHost() const
{
    return(_host);
}
fileExtension RequestParser::getType() const
{
    return(_type);
}


/**************************************************************/
/*                      PUBLIC METHODS                        */
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
    std::cout << "----------Buffer: " << std::endl << buffer << std::endl;
    //splitBuffer()
    std::stringstream linestream(buffer);
    std::string method;
	std::string lineSkip;		// fbecht changed here
    std::getline(linestream, method, ' ');
	removeCarriageReturn(method);
    if (method == "GET")
        _method = GET;
    else if (method == "POST")
        _method = POST;
    else if (method == "DELETE")
        _method = DELETE;
    else if (method == "HEAD")
        _method = HEAD;
    else if (method == "PUT")
        _method = PUT;
    else if (method == "OPTIONS")
        _method = OPTIONS;
    else if (method == "TRACE")
        _method = TRACE;
    else if (method == "CONNECT")
        _method = CONNECT;
    else
        _method = NO_TYPE;
    // if (_method != "GET" && _method != "POST" && _method != "DELETE")
    //     throw std::runtime_error("Error: invalid request type");
    std::getline(linestream, _path, ' ');
    if (_path[0] != '/')
        throw std::runtime_error("Error: invalid path in request");
    if (_path.find("?") != std::string::npos)
    {
        std::stringstream sstream(_path);
        std::getline(sstream, _path, '?');
        std::getline(sstream, _query);
    }
    if (_path.find(".") != std::string::npos)
    {
        char c1, c2;
        std::string type;
        std::istringstream sstream(_path);
        size_t dotPos = _path.find(".");
        sstream.seekg(dotPos + 1);
        sstream >> c1 >> c2; type = c1; type += c2;
        if (type == "cs")
            _type = CSS;
        else if (type == "ph")
            _type = PHP;
        else if (type == "py")
            _type = PY;
        else
            _type = HTML;
    }
    std::getline(linestream, _protocol, '\r');
	std::getline(linestream, lineSkip);		// fbecht changed here
	removeCarriageReturn(_protocol);
    std::string key;
    std::string value;
    while (linestream)
    {
        std::getline(linestream, key, ':');
        if (key[0] == '\r' && (_method == POST || _method == DELETE))
        {
            key.erase(0, 2);
            _body = key;
            break ;
        }
        if (key.empty() || key[0] == '\r')
            break ;
        //removeCarriageReturn(key);
        std::getline(linestream, value);
		removeCarriageReturn(value);
		std::getline(linestream, lineSkip);		// fbecht changed here
        if (value.empty())
            break ;
        value = value.erase(0, 1);
        _headers[key] = value;
    }
    _host = _headers["Host"];
    std::cout << "---------------------------------All headers: " << std::endl;
    std::map<std::string, std::string>::iterator it;
    it = _headers.begin();
    for (it = _headers.begin(); it != _headers.end(); ++it)
        std::cout << it->first << " " << it->second << std::endl;
}

void RequestParser::parseHeaders()
{
    
}


/**************************************************************/
/*                    OVERLOAD OPERATOR                       */
/**************************************************************/

std::ostream &operator<<(std::ostream &str, RequestParser &rp)
{
    str << "Method: " << rp.getMethod() << std::endl;
    str << "Path: " << rp.getPath() << std::endl;
    str << "Query: " << rp.getQuery() << std::endl;
    str << "Type: " << rp.getType() << std::endl;
    str << "Protocol: " << rp.getProtocol() << std::endl;
    str << "Host: " << rp.getHost() << std::endl;
    str << "Body: " << rp.getBody() <<std::endl;
    //str << "Headers: ";
    // std::map<std::string, std::string>::iterator it;
    // for (it = rp.getHeaders().begin(); it != rp.getHeaders().end(); ++it)
    //     str << it->first << " " << it->second << std::endl;
    return (str);
};
