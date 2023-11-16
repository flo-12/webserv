/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 12:52:22 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/16 12:54:43 by pdelanno         ###   ########.fr       */
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
    _contentType = "";
    _isUpload = false;
    _boundaryCode = "";
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
    _contentType = "";
    _isUpload = false;
    _boundaryCode = "";
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
RequestParser::formObject RequestParser::getFormObject() const
{
    return(_form);
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

ssize_t &RequestParser::_getFormBodyLength(ssize_t &bodyLength)
{
    std::cout << YELLOW << bodyLength << RESET_PRINT << std::endl;
    ssize_t contentLength = std::atol(_headers["Content-Length"].c_str());
    _boundaryCode.size();
    std::cout << contentLength << std::endl;
    //###########
    return (bodyLength);
}

void RequestParser::parseRequest(std::string const &buffer)
{
    //splitBuffer()
    std::stringstream linestream(buffer);
    std::string method;
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
    std::getline(linestream, _protocol, '\n');
	removeCarriageReturn(_protocol);
    std::string key;
    std::string value;
    while (linestream)
    {
        std::getline(linestream, key, ':');
        if (key[0] == '\r' && (_method == POST || _method == DELETE))
        {
            key.erase(0, 2);
            if (_isUpload == false)
                _body = key;
            else
            {
                // retrieve boundary code from content-type!
                std::stringstream sboundary(key);
                std::getline(sboundary, _boundaryCode, '\n');
                //_boundaryCode.erase(0, 1);
                std::string formBuffer;

                std::getline(linestream, formBuffer, '\r');
                
                std::stringstream sbuffer(formBuffer);
                std::getline(sbuffer, key, '=');
                std::getline(sbuffer, _form.name, ';');
                _form.name.erase(0, 1);
                _form.name.erase(_form.name.length() - 1, _form.name.length());
                std::getline(sbuffer, key, '=');
                if (!sbuffer.good())
                    ;
                else
                {
                    std::getline(sbuffer, _form.fileName, ';');
                    _form.fileName.erase(0, 1);
                    _form.fileName.erase(_form.fileName.length() - 1, _form.fileName.length());
                    std::cout << YELLOW << _form.name << RESET_PRINT << std::endl;
                    std::getline(linestream, formBuffer, '\n');
                    std::getline(linestream, formBuffer, '\n');
                    std::getline(linestream, formBuffer, '\n');
                    std::getline(linestream, _form.body, '\r');
                    _form.bodyLength = _form.body.size();
                    _getFormBodyLength(_form.bodyLength);
                    std::getline(linestream, formBuffer, '\r');
                    std::getline(linestream, _body, '\r');
                }
                _body.erase(0, 1);
            }
            break ;
        }
        if (key.empty() || key[0] == '\r')
        {
            break ;
        }
        std::getline(linestream, value);
		removeCarriageReturn(value);
        if (value.empty())
            break ;
        value = value.erase(0, 1);
        if (key == "Content-Type" && value.find("multipart/form-data") != std::string::npos)
            _isUpload = true;
        _headers[key] = value;
    }
    
    _host = _headers["Host"];
    std::stringstream sstream(_host);
    std::string localhost;
    std::getline(sstream, localhost, ':');
    if (localhost == "localhost")
        _host = "127.0.0.1:18000";
    
    // _contentType = _headers["Content-Type"];
    // sstream.str(_contentType);
    // std::string contentType;
    // std::getline(sstream, contentType, ';');
    // if (contentType == "multipart/form-data")
    // {
    //     std::string boundary;
    //     std::getline(sstream, boundary, '\n');
    //     boundary.erase(0, 1);
    //     _handleBoundaries(boundary);
    // }
    // else
    //     std::cout << "popola" << std::endl;
    
    // std::cout << "---------------------------------All headers: " << std::endl;
    // std::map<std::string, std::string>::iterator it;
    // it = _headers.begin();
    // for (it = _headers.begin(); it != _headers.end(); ++it)
    //     std::cout << it->first << " " << it->second << std::endl;
    // std::cout << "----------xxxx---------";
}

void RequestParser::_handleBoundaries(std::string boundary)
{
    std::cout << boundary << std::endl;
    std::cout << "body is: " << _body << std::endl;
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
