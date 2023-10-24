/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socketCom.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/23 18:06:16 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/23 18:06:18 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"


/**************************************************************/
/*                      CANONICAL FORM                        */
/**************************************************************/

WebServ::WebServ()
	: _serverPorts(serverPorts), _serverIp(serverIPs), _nbrServers(nbrServers), 
	_maxConnections(MAX_CONNECTIONS), _maxGetSize(MAX_GET_SIZE)
{
	// Initialize all FD with -1
	memset( _serverFds, -1, sizeof(_serverFds) );

	// Initialize all servers
	for( int i = 0; i < _nbrServers; ++i ) {
		_setupServer( i );
		_initFdSet( i );
	}

	std::cout << "server initialized & listening on port" << std::endl;
}

WebServ::~WebServ()
{
	serverShutdown();
}

/**************************************************************/
/*                     PRIVATE METHODS                        */
/**************************************************************/

void	WebServ::_setupServer( int i )
{
	// Create socket
	if ( (_serverFds[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		throw std::runtime_error("Error: setup server (socket() failed)");

	// Reusable Port and Socket
	int on = 1;
	if ( setsockopt(_serverFds[i], SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT,
				&on, sizeof(int)) < 0 )
		throw std::runtime_error("Error: setup server (setsockopt() failed)");

	// Set socket to non-blocking
	if( fcntl(_serverFds[i], F_SETFL, O_NONBLOCK) < 0 )
		throw std::runtime_error("Error: setup server (fcntl() failed)");

	// Identify a socket (assigning transport address to socket)
	struct sockaddr_in	server_addr;

	memset( &server_addr, 0, sizeof(server_addr) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(_serverIp[i]);
	server_addr.sin_port = htons(_serverPorts[i]);
	if ( bind(_serverFds[i], (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
		throw std::runtime_error("Error: setup server (bind() failed)");

	// Listen for connections on a socket (mark socket as passive)
	if( listen(_serverFds[i], _maxConnections) < 0 )
		throw std::runtime_error("Error: setup server (listen() failed)");
}

void	WebServ::_initFdSet( int i )
{
	_fdSet[i].fd = _serverFds[i];	// set descriptor fd to to listening socket
	_fdSet[i].events = 0;			// Clear the bit array
	_fdSet[i].events = _fdSet[i].events | POLLIN;	// Set the POLLIN bit	
}

int	WebServ::_acceptNewConnection( int serverFd, int nbrFd )
{
	int	clientFd;
	
	errno = 0;

	clientFd = accept(serverFd, NULL, NULL);
	if ( clientFd < 0 && errno != EWOULDBLOCK )
	{
		std::cerr << "Error: accept() failed" << std::endl;
		return nbrFd;
	}

	if( fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0 )
	{
		std::cerr << "Error: fcntl() for client failed" << std::endl;
		return -1;
	}

	_fdSet[nbrFd].fd = clientFd;
	_fdSet[nbrFd].events = POLLIN;
	_fdSet[nbrFd].revents = 0;
	nbrFd++;

	std::cout << "\r" << "New connection with client " << clientFd << "established." << std::endl;

	return nbrFd;
}

void	WebServ::_receiveRequest( int clientFd, int i )
{
	char	buffer[_maxGetSize];
	ssize_t	bytesRead;

	if ( (bytesRead = recv(clientFd, &buffer, _maxGetSize - 1, 0)) < 0 )
		throw std::runtime_error("Error: recv() failed");	// tbd: handle all errors -> really exit program?
	else
		buffer[bytesRead] = '\0';
	fflush( stdout );

	if ( bytesRead < _maxGetSize - 1 )
	{
		_fdSet[i].events = POLLOUT;
		std::cout << "Request received complete" << std::endl;
		std::cout << buffer << std::endl;
	}
	else	// tbd: handle request bigger than buffer (store in internal std::string)
		std::cout << "Request received incomplete (buffer not big enough)" << std::endl;
}

int	WebServ::_sendResponse( int clientFd, int nbrFd, int i )
{
	std::cout << "\r" << "Sending Response on client: " << _fdSet[i].fd << std::endl;

	if ( send(clientFd, _responseBuilder().c_str(), strlen(_responseBuilder().c_str()), 0) < 0 )
		std::cerr << "Error: handle connection (send() failed)" << std::endl;
	
	// close the socket and remove it from the fdSet
	close( _fdSet[i].fd );
	_fdSet[i] = _fdSet[nbrFd - 1];
	//_fdSet[nbrFd - 1] = {0, 0, 0}; // change _fdSet to std::vector and do _fdSet.pop_back()
	nbrFd--;

	return nbrFd;
}

std::string	WebServ::_responseBuilder()
{
	std::string	crlf = "\r\n";
	std::string	response;

	response += "HTTP/1.1 200 OK" + crlf;			// Status-Line
	response += "Content-Type: text/html" + crlf;	// Entity-Header-Field
	response += "Content-Length: 12" + crlf;		// Entity-Header-Field
	response += crlf;
	response += "Hello World" + crlf;				// message-body
	response += crlf;
	
	return response;
}

bool	WebServ::_fdIsServer( int fdToFind )
{
	for ( int i = 0; i < _nbrServers; ++i )
	{
		if ( _serverFds[i] == fdToFind )
			return true;
	}
	return false;
}

/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

void	WebServ::serverRun()
{
	std::cout << "server running and responding..." << std::endl;

	int	timeout = TIMEOUT_POLL;
	int	retPoll = 0;

	int	nbrFd = _nbrServers;

	while (42)
	{
		// Verify if a new connection is available
		if ( (retPoll = poll(_fdSet, nbrFd, timeout)) < 0)
			throw std::runtime_error("Error: poll() failed");
		
		// Verify which server has a new connection
		while ( retPoll > 0 ) {
			for (int i = 0; i < nbrFd; ++i)
			{
				if ( (_fdSet[i].revents & POLLERR) == POLLERR )	// tbd: handle all errors!!!!
					break ;
				else if ( (_fdSet[i].revents & POLLIN) == POLLIN && _fdIsServer(_fdSet[i].fd) )
					nbrFd = _acceptNewConnection(_fdSet[i].fd, nbrFd);
				else if ( (_fdSet[i].revents & POLLIN) == POLLIN )
					this->_receiveRequest( _fdSet[i].fd, i );
				else if ( (_fdSet[i].revents & POLLOUT) == POLLOUT )
					nbrFd = this->_sendResponse( _fdSet[i].fd, nbrFd, i );
			}
			retPoll--;
		}
	}
}

void	WebServ::serverShutdown()
{
	std::cout << "server shutting down..." << std::endl;

	for( int i = 0; i < _nbrServers; ++i ) {
		if ( _serverFds[i] != -1 )
			close(_serverFds[i]);
	}
}
