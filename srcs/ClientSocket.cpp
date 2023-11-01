/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 11:13:14 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/31 11:13:16 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientSocket.hpp"


/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

ClientSocket::ClientSocket( int serverFd )
	: Socket( CLIENT ), _serverFd(serverFd)
{
	_startTime = time(0);
	_clearRequest();
	_clearResponse();
}

ClientSocket::~ClientSocket()
{
	std::cout << "ClientSocket destructor (fd: " << _fd << ", serverFd: " << _serverFd << ")" << std::endl;
}


/**************************************************************/
/*                     PRIVATE METHODS                        */
/**************************************************************/

/* setupSocket:
*	Accepts a new connection and sets the socket to non-blocking.
*/
void	ClientSocket::setupSocket()
{
	_fd = accept( _serverFd, NULL, NULL );

	if ( _fd < 0 )
		throw std::runtime_error("Error: _acceptNewConnection (accept() failed).");

	if ( fcntl(_fd, F_SETFL, O_NONBLOCK) < 0 )	// set socket to non-blocking
		throw std::runtime_error("Error: _acceptNewConnection (fcntl() failed).");
}

/* _clearRequest:
*	Clears the _request struct.
*/
void	ClientSocket::_clearRequest()
{
	_request.startTime = time(0);
	_request.pendingReceive = false;
	_request.contentLength = 0;
	_request.readBytes = 0;
	_request.buffer.clear();
}

/* _clearResponse:
*	Clears the _response struct.
*/
void	ClientSocket::_clearResponse()
{
	_response.startTime = time(0);
	_response.pendingSend = false;
	_response.contentLength = 0;
	_response.sentBytes = 0;
	_response.header.clear();
	_response.body.clear();
}


/* bodyComplete:
*	Checks if the body of the request is complete (received).
*	Also sets the _request.contentLength.
*	Returns: true if the body is complete, false otherwise.
*/
bool	ClientSocket::_requestComplete()
{
	size_t	cLen = _request.buffer.find("Content-Length: ");
	size_t	endHeader = _request.buffer.find("\r\n\r\n"); // +4?!

	if ( cLen == std::string::npos || endHeader == std::string::npos )
		_request.contentLength = 0;
	else
		_request.contentLength = static_cast<ssize_t>(atoi( _request.buffer.substr(cLen + 16, _request.readBytes).c_str() ));
	
	if ( _request.readBytes >= _request.contentLength + static_cast<ssize_t>(endHeader + 4) )
		return true;
	else
		return false;
}


/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

/* closeConnection:
*	Sends the error page matching the HttpErrorType passed as argument.
*/
void	ClientSocket::closeConnection( HttpErrorType httpError )
{
	std::cout << "ClientSocket::closeConnection() with httpError: " << httpError << std::endl;
}

/* hasTimeout:
*	Checks if the client has timed out.
*	In case that startTime is -1 (time() failed), the client is
*	considered as timed out.
*	Returns: true if the client has timed out, false otherwise.
*/
bool	ClientSocket::hasTimeout()
{
	if ( _startTime == -1 || _request.startTime == -1 )
		return true;
	else if ( time(0) - _startTime > TIMEOUT_RECEIVE )
		return true;
	else
		return false;
}


/* receiveRequest:
*	Receives the request from the client and stores it in _request.
*	Also checks if the request is complete or if errors occured.
*	Returns: the status of the receive operation.
*
*	Further versions:
*		- handle POST request (content-length, chunked transfer encoding)
*		- handle request bigger than buffer
*		- handle errno of recv() (EAGAIN, EWOULDBLOCK, EINTR) <--- DO WE NEED THAT???
*/
ReceiveStatus	ClientSocket::receiveRequest()
{
	char	buffer[MAX_REQ_SIZE] = {0};
	ssize_t	bytesRead;

	// Update startTime
	_request.startTime = time(0);

	// Receive request
	if ( (bytesRead = recv(_fd, &buffer, MAX_REQ_SIZE, O_NONBLOCK)) < 0 )
		return READ_ERROR;
	else if (bytesRead == 0)
		return CLIENT_CLOSED;
	fflush( stdout );

	_request.readBytes += bytesRead;
	_request.buffer.append( std::string(buffer, bytesRead) );

	// Check if request is complete and set Content-Length
	if ( _requestComplete() )
	{
		/* std::cout << "+++++++++++++++++ Request +++++++++++++++++" << std::endl;
		std::cout << _request.buffer << std::endl;
		std::cout << "+++++++++++++++++++++++++++++++++++++++++++" << std::endl; */
		//_saveFile( "./html/surfer_POST.jpeg", _request.buffer );

		_request.pendingReceive = false;
		return READ_DONE;
	}
	else
	{
		_request.pendingReceive = true;
		return READ_PENDING;
	}
}

/* sendResponse:
*	Sends the response to the client.
*/
ResponseStatus	ClientSocket::sendResponse()
{
	// Update startTime
	_response.startTime = time(0);

	//std::cout << "+++++++++++++++ Request Parser +++++++++++++++" << std::endl;
	RequestParser	requestParser( _request.buffer );
	//std::cout << requestParser << std::endl;
	//std::cout << "++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

	_buildResponse( requestParser.getPath() );
	//_buildResponseCGI( requestParser.getPath() );
	std::string	msgSend = _response.header + _response.body;
	
	ssize_t	bytesSent = send(_fd, msgSend.c_str(), msgSend.length(), 0);
	//ssize_t	bytesSent = send(_fd, msgSend.c_str(), _request.contentLength, 0);
	if ( bytesSent < static_cast<ssize_t>(-1) )
		return SEND_ERROR;

	_response.sentBytes += bytesSent;

	if ( _response.sentBytes >= _response.contentLength )
	{
		_response.pendingSend = false;
		_clearRequest();
		_clearResponse();
				
		return SEND_DONE;
	}
	else
	{
		_response.pendingSend = true;

		return SEND_PENDING;
	}
}


/**************************************************************/
/*                    RESPONSE PARSER                         */
/**************************************************************/

/* buildResponse():
*	Built the response corresponding to the _request and stores it in _response.
*	1st draft contains:
*		- filling out all values from _request
*		- "Hello World" in the body
*		- status line ("HTTP/1.1 200 OK"), "Content-Length" (text/html) and "Content-Length" in the header
*/
void	ClientSocket::_buildResponse( std::string path )
{
	std::string prefix1 = "/";
	std::string prefix2 = "/surfer.jpeg";
	std::string prefix3 = "/giga-chad-theme.mp3";

	//if (_request.buffer.find(prefix1) == 0)
	if ( path == prefix1 )
	{
		_response.body = _readFile( "./html" + path + "index.html" );
		//_response.body = "Hello World\n";
		std::stringstream	strStream;
		strStream << (_response.body.length());

		_response.header = "HTTP/1.1 200 OK\r\n";
		_response.header += "Content-Type: text/html\r\n";
		_response.header += "Content-Length: " + strStream.str() + "\r\n";
		_response.header += "Connection: close\r\n";
		_response.header += "\r\n";
	}
	else if ( path == prefix2 )
	{
		_response.body = _readFile( "./html" + path );
		std::stringstream	strStream;
		strStream << (_response.body.length());

		_response.header = "HTTP/1.1 200 OK\r\n";
		_response.header += "Content-Type: image/*\r\n";
		_response.header += "Content-Length: " + strStream.str() + "\r\n";
		_response.header += "Connection: close\r\n";
		_response.header += "\r\n";
	}
	else if (  path == prefix3 )
	{
		_response.body = _readFile( "./html" + path );
		std::stringstream	strStream;
		strStream << (_response.body.length());

		_response.header = "HTTP/1.1 200 OK\r\n";
		_response.header += "Content-Type: audio/*\r\n";
		_response.header += "Content-Length: " + strStream.str() + "\r\n";
		_response.header += "Connection: close\r\n";
		_response.header += "\r\n";
	}
	_response.contentLength = _response.header.length() + _response.body.length();
}

/* FOR CGI-TESTING */
void	ClientSocket::_buildResponseCGI( std::string path )
{
	path = "";
	std::string output;
	CGIHandler CGIHandler;
	output = CGIHandler.execute();
	_response.body = output;
	std::stringstream	strStream;
	strStream << (_response.body.length());

	_response.header = "HTTP/1.1 200 OK\r\n";
	_response.header += "Content-Type: text/html\r\n";
	_response.header += "Content-Length: " + strStream.str() + "\r\n";
	_response.header += "\r\n";
	_response.contentLength = _response.header.length() + _response.body.length();
}


std::string	ClientSocket::_readFile( std::string path )
{
	std::cout << "Reading file: " << path << std::endl;
	std::string		line;
	std::string		body;
	std::ifstream	ifs(path.c_str());

	if ( !ifs.is_open() )
		throw std::runtime_error("Error: readFile() failed");

	while ( getline( ifs, line ) )
		body += line + "\n";
	ifs.close();

	return body;
}

void	ClientSocket::_saveFile( std::string path, std::string content )
{
	std::cout << "Saving file: " << path << std::endl;
	std::ofstream	ofs(path.c_str());

	if ( !ofs.is_open() )
		throw std::runtime_error("Error: saveFile() failed");

	ofs << content;
	ofs.close();
}
