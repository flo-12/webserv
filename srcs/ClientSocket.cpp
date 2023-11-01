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
	_response.pendingSend = false;
	_response.contentLength = 0;
	_response.sentBytes = 0;
	_response.header.clear();
	_response.body.clear();
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

	// Receive request
	if ( (bytesRead = recv(_fd, &buffer, MAX_REQ_SIZE, O_NONBLOCK)) < 0 )
		return READ_ERROR;
	else if (bytesRead == 0)
		return CLIENT_CLOSED;
	fflush( stdout );

	// Check if request is complete		<-- HAS TO BE REVISED!!!!!!!!
	size_t	found = std::string(buffer).find("\r\n\r\n");
	if ( found != std::string::npos )
	{
		_request.pendingReceive = false;
		//clientSocket._request.contentLength = 0;	// <-- HAS TO BE REVISED (FOR POST)!!!!!!!!
		_request.readBytes += bytesRead;
		_request.buffer += buffer;

		std::cout << "+++++++++++++++ Request +++++++++++++++" << std::endl;
		std::cout << _request.buffer << std::endl;
		std::cout << "++++++++++++++++++++++++++++++++++++++++" << std::endl;

		return READ_DONE;
	}
	else
		return READ_PENDING;
}

/* sendResponse:
*	Sends the response to the client.
*/
ResponseStatus	ClientSocket::sendResponse()
{
	_buildResponse();
	std::string	msgSend = _response.header + _response.body;
	
	ssize_t	bytesSent = send(_fd, msgSend.c_str(), msgSend.length(), 0);
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
void	ClientSocket::_buildResponse()
{
	std::string prefix1 = "GET / HTTP/1.1";
	std::string prefix2 = "GET /surfer.jpeg HTTP/1.1";
	std::string prefix3 = "GET /giga-chad-theme.mp3 HTTP/1.1";

	if (_request.buffer.find(prefix1) == 0)
	{
		_response.body = _readFile("./html/index.html");
		//_response.body = "Hello World\n";
		std::stringstream	strStream;
		strStream << (_response.body.length());

		_response.header = "HTTP/1.1 200 OK\r\n";
		_response.header += "Content-Type: text/html\r\n";
		_response.header += "Content-Length: " + strStream.str() + "\r\n";
		_response.header += "\r\n";
	}
	else if (_request.buffer.find(prefix2) == 0)
	{
		_response.body = _readFile("./html/surfer.jpeg");
		std::stringstream	strStream;
		strStream << (_response.body.length());

		_response.header = "HTTP/1.1 200 OK\r\n";
		_response.header += "Content-Type: image/*\r\n";
		_response.header += "Content-Length: " + strStream.str() + "\r\n";
		_response.header += "\r\n";
	}
	else if (_request.buffer.find(prefix3) == 0)
	{
		_response.body = _readFile("./html/giga-chad-theme.mp3");
		std::stringstream	strStream;
		strStream << (_response.body.length());

		_response.header = "HTTP/1.1 200 OK\r\n";
		_response.header += "Content-Type: audio/*\r\n";
		_response.header += "Content-Length: " + strStream.str() + "\r\n";
		_response.header += "\r\n";
	}
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