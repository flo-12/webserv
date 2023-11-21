/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 12:52:22 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/21 14:30:55 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/RequestParser.hpp"

/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

RequestParser::RequestParser() {}

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
ssize_t RequestParser::getBodyLength() const
{
    return(_bodyLength);
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
    ssize_t contentLength = std::atol(_headers["Content-Length"].c_str());
    bodyLength = contentLength - (((_boundaryCode.size() + 2) * 2) + 2);
    bodyLength = bodyLength - (52 + _form.name.size() + _form.fileName.size()); // 52 characters in the content-disposition header
    bodyLength = bodyLength - (14 + _form.contentType.size()); // 14 characters for content-type
    bodyLength = bodyLength - 12; // trailing /n & /r
    return (bodyLength);
}

void RequestParser::_processForm(std::stringstream &linestream)
{
    _boundaryCode = _headers["Content-Type"];
    _boundaryCode.erase(0, 30); // 30 characters to boundary
    std::string key;
    
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
        std::getline(linestream, formBuffer, '\n');
        std::getline(linestream, _form.contentType, '\n');
        _form.contentType.erase(0, 14);
        _form.contentType.erase(_form.contentType.length() - 1, _form.contentType.length());
        std::getline(linestream, formBuffer, '\n');
        _getFormBodyLength(_form.bodyLength);
		_bodyLength = _form.bodyLength;
        
        char* buffer = new char[_form.bodyLength + 1];
        linestream.read(buffer, _form.bodyLength);
        buffer[_form.bodyLength] = '\0';
        _form.body = std::string(buffer, _form.bodyLength);
        delete[] buffer;
        
        std::getline(linestream, formBuffer, '\r');
        std::getline(linestream, _body, '\r');
    }
    _body.erase(0, 1);
}

void RequestParser::parseRequest(std::string const &buffer)
{
    std::stringstream linestream(buffer);

    // retrieve method

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

    // retrieve path and query

    std::getline(linestream, _path, ' ');
    if (_path[0] != '/')
        throw std::runtime_error("Error: invalid path in request");
    if (_path.find("?") != std::string::npos)
    {
        std::stringstream sstream(_path);
        std::getline(sstream, _path, '?');
        std::getline(sstream, _query);
    }

    // check file extension for CGI
    
    if (_path.find(".") != std::string::npos)
    {
        char c1, c2;
        std::string type;
        std::istringstream sstream(_path);
        size_t dotPos = _path.find(".");
        sstream.seekg(dotPos + 1);
        sstream >> c1 >> c2; type = c1; type += c2;
        if (type == "ph")
            _type = PHP;
        else if (type == "py")
            _type = PY;
        else
            _type = OTHER;
    }

    // retrieve protocol

    std::getline(linestream, _protocol, '\n');
	removeCarriageReturn(_protocol);

    // retrieve headers, body and form in case of upload

    std::string key;
    std::string value;
    while (linestream)
    {
        std::getline(linestream, key, ':');
        if (key[0] == '\r' && (_method == POST || _method == DELETE))
        {
            key.erase(0, 2);
            if (_isUpload == false) {
				_body = key;
				_bodyLength = _body.length();
			}
            else
                _processForm(linestream);
            break ;
        }
        if (key.empty() || key[0] == '\r')
            break ;
        std::getline(linestream, value);
		removeCarriageReturn(value);
        if (value.empty())
            break ;
        value = value.erase(0, 1);
        if (key == "Content-Type" && value.find("multipart/form-data") != std::string::npos)
            _isUpload = true;
        _headers[key] = value;
    }

    // retrieve host

    _host = _headers["Host"];

    // print headers

    printDebug("------Request headers------", DEBUG_REQUEST_HEADER, BABY_BLUE, 0);
    if (DEBUG_REQUEST_HEADER == 1)
    {
        std::map<std::string, std::string>::iterator it;
        it = _headers.begin();
        for (it = _headers.begin(); it != _headers.end(); ++it)
            std::cout << BABY_BLUE << it->first << " " << it->second << RESET_PRINT << std::endl;
    }
    printDebug("---------------------------", DEBUG_REQUEST_HEADER, BABY_BLUE, 0);
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
    return (str);
};
