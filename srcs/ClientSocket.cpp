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

ClientSocket::ClientSocket( int serverFd, ServerConfig config )
	: Socket( CLIENT, config ), _serverFd(serverFd)
{
	_startTime = time(0);
	_clearRequest();
	_clearResponse();
}

ClientSocket::~ClientSocket()
{
	//std::cout << "ClientSocket destructor (fd: " << _fd << ", serverFd: " << _serverFd << ")" << std::endl;
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
	_response.startSending = false;
	_response.pendingSend = false;
	_response.msgLength = 0;
	_response.sentBytes = 0;
	_response.message.clear();
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
*	Sends the error page matching the HttpStatusCode passed as argument.
*/
void	ClientSocket::closeConnection( HttpStatusCode httpStatus )
{
	if ( httpStatus != NO_ERROR )
	{
		try
		{
			Response	response( httpStatus );	// to be implemented
	
			_response.message = response.getResponseMsg();
			_response.msgLength = response.getMsgLength();
		}
		catch( const std::exception& e )
		{
			_response.message = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
			_response.msgLength = _response.message.length();
			std::cerr << e.what() << '\n';
		}
		_response.startSending = true;
		sendResponse();
	}

	std::cout << "\tClientSocket::closeConnection() with httpStatus: " << httpStatus << "\n" << std::endl;
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
	if ( !_response.startSending ) {
		_response.startTime = time(0);

		RequestParser	requestParser( _request.buffer, _request.contentLength );
	
		try
		{
			Response	response( requestParser, _config );

			_response.message = response.getResponseMsg();
			_response.msgLength = response.getMsgLength();
		}
		catch( const std::exception& e )
		{
			_response.message = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
			_response.msgLength = _response.message.length();
			std::cerr << e.what() << '\n';
		}
		_response.startSending = true;
	}

	/* std::cout << "+++++++++++++++++ Response +++++++++++++++++" << std::endl;
	std::cout << _response.message << std::endl;
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++" << std::endl; */

	ssize_t	bytesSent = send(_fd, _response.message.c_str(), _response.msgLength, 0);
	if ( bytesSent < static_cast<ssize_t>(-1) )
		return SEND_ERROR;

	_response.sentBytes += bytesSent;

	if ( _response.sentBytes >= _response.msgLength )
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

