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

	// Initialize all servers and the _pollFds
	_nbrFds = 0;
	for ( int i = 0; i < _nbrServers; ++i ) {
		_setupServer( i );
		_initFdSet( _serverFds[i] );
		_nbrFds++;
	}
	for ( int i = _nbrServers; i < _maxConnections; ++i )
		_pollFds[i].fd = -1;
	
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

void	WebServ::_initFdSet( int fd )
{
	_pollFds[_nbrFds].fd = fd;		// set descriptor fd to to listening socket
	_pollFds[_nbrFds].events = 0;	// Clear the bit array
	_pollFds[_nbrFds].events = _pollFds[_nbrFds].events | POLLIN;	// Set the POLLIN bit	
}

void	WebServ::_acceptNewConnection( int serverFd )
{
	int	clientFd;
	
	errno = 0;

	clientFd = accept(serverFd, NULL, NULL);
	if ( clientFd < 0 && errno != EWOULDBLOCK )
	{
		std::cerr << "Error: accept() failed" << std::endl;
		return ;
	}

	if( fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0 )
	{
		std::cerr << "Error: fcntl() for client failed" << std::endl;
		return ;
	}

	_pollFds[_nbrFds].fd = clientFd;
	_pollFds[_nbrFds].events = POLLIN;
	_pollFds[_nbrFds].revents = 0;
	_nbrFds++;

	std::cout << "New connection with client " << clientFd << " established." << std::endl;
}

void	WebServ::_receiveRequest( struct pollfd *client )
{
	char	buffer[_maxGetSize];
	ssize_t	bytesRead;

	if ( (bytesRead = recv(client->fd, &buffer, _maxGetSize - 1, 0)) < 0 )
		throw std::runtime_error("Error: recv() failed");	// tbd: handle all errors -> really exit program?
	else
		buffer[bytesRead] = '\0';
	fflush( stdout );

	/* TBD: review!!! (case return 0 should close the fd?!) */
	if ( bytesRead < _maxGetSize - 1 )
	{
		client->events = POLLOUT;
		std::cout << "Request received complete" << std::endl;
		//std::cout << buffer << std::endl;
	}
	else	// tbd: handle request bigger than buffer (store in internal std::string)
		std::cout << "Request received incomplete (buffer not big enough)" << std::endl;
}

void	WebServ::_sendResponse( struct pollfd *client, unsigned int i )
{
	std::cout << "Sending Response on client: " << client->fd << std::endl;

	if ( send(client->fd, _responseBuilder().c_str(), strlen(_responseBuilder().c_str()), 0) < 0 )
		std::cerr << "Error: handle connection (send() failed)" << std::endl;
	
	// close the socket and remove it from the fdSet
	close( client->fd );
	_pollFds[i] = _pollFds[_nbrFds - 1];
	_pollFds[_nbrFds - 1].fd = -1; // TBD: Review if OK
	_nbrFds--;
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

	while (42)
	{
		// Verify if a new connection is available
		if ( (retPoll = poll(_pollFds, _nbrFds, timeout)) < 0)
			throw std::runtime_error("Error: poll() failed");
	
		// Verify which server has a new connection
		while ( retPoll > 0 ) {
			for (unsigned int i = 0; i < _nbrFds; ++i)
			{
				if ( (_pollFds[i].revents & POLLERR) == POLLERR )	// tbd: handle all errors!!!!
					break ;
				else if ( (_pollFds[i].revents & POLLIN) == POLLIN && _fdIsServer(_pollFds[i].fd) )
					_acceptNewConnection(_pollFds[i].fd );
				else if ( (_pollFds[i].revents & POLLIN) == POLLIN )
					_receiveRequest( &_pollFds[i] );
				else if ( (_pollFds[i].revents & POLLOUT) == POLLOUT )
					_sendResponse( &_pollFds[i], i );
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
