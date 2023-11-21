/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 11:13:07 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/31 11:13:09 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerSocket.hpp"

/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

ServerSocket::ServerSocket( unsigned int ip, int port, ServerConfig config )
	: Socket( SERVER, config ), _ip(ip), _port(port), _nbrRestarts(0)
{
	setupSocket();
}

ServerSocket::~ServerSocket()
{
}


/**************************************************************/
/*                     PRIVATE METHODS                        */
/**************************************************************/

/* _setupServer:
*	Create a new Socket for a server:
*		- create socket
*		- set socket options (reusable port and socket & non-blocking)
*		- bind socket to port
*		- listen on socket
*		- add socket fd to _pollFds
*	Return: the created socket.
*/
void	ServerSocket::setupSocket()
{
	// Create socket
	if ( (_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		throw std::runtime_error("Error: createServerSocket (socket() failed)");

	// Reusable Port and Socket
	int on = 1;
	if ( setsockopt(_fd, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT,
				&on, sizeof(int)) < 0 )
		throw std::runtime_error("Error: createServerSocket (setsockopt() failed)");

	// Set socket to non-blocking
	if( fcntl(_fd, F_SETFL, O_NONBLOCK) < 0 )
		throw std::runtime_error("Error: createServerSocket (fcntl() failed)");
	
	// Identify a socket (assigning transport address to socket)
	struct sockaddr_in	server_addr;

	memset( &server_addr, 0, sizeof(server_addr) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(_ip);
	server_addr.sin_port = htons(_port);
	if ( bind(_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
		throw std::runtime_error("Error: setup server (bind() failed)");

	// Listen for connections on a socket (mark socket as passive)
	if( listen(_fd, MAX_CONNECTIONS) < 0 )
		throw std::runtime_error("Error: setup server (listen() failed)");
}


/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

/* restartServerSocket:
*	Restart server Socket.
*		- close socket fd
*		- create socket new
*/
void	ServerSocket::restartServerSocket()
{
	if ( _fd > 0 )
		close( _fd );
	setupSocket( );
	_nbrRestarts++;
}
