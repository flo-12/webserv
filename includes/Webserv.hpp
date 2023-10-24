/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socketCom.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/23 18:06:08 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/23 18:06:11 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
//#include <cstdlib> // for exit()
#include <sys/socket.h> // for socket()
#include <netdb.h> // for struct sockaddr_in
#include <string.h> // for bzero(), strlen()
#include <unistd.h> // for close(), read(), write()
#include <stdio.h> // for printf(), snprintf(), fflush()
#include <fcntl.h> // for open(), O_RDONLY
#include <cerrno> // for errno
#include <poll.h> // for poll
#include <exception> // for exception
#include <vector> // for vector

/************ INTERFACE TO CONFIG_PARSER ************/
# define MAX_GET_SIZE 8192
# define MAX_CONNECTIONS 2000
#define TIMEOUT_POLL 500

const int serverIPs[] = {2130706433, 2130706433, 2130706434}; 	/* 127.0.0.1 converted to int */
const int serverPorts[] = {18000, 20000, 19000};
const int nbrServers = sizeof(serverPorts) / sizeof(serverPorts[0]);


/* Class to run a TCP-Server
*	- default constructor initializes the server attributes and sets up the socket
*	- serverRun() to start the server and being able to connect with it
*/
class WebServ
{
	private:
		const int*		_serverPorts;
		const int*		_serverIp;
		const int		_nbrServers;
		unsigned int	_nbrFds;
		const int		_maxConnections;
		const int		_maxGetSize;
		int				_serverFds[nbrServers];
		struct pollfd	_pollFds[MAX_CONNECTIONS];

		void	_setupServer( int i );
		void	_initFdSet( int fd );

		void		_acceptNewConnection( int serverFd );
		void		_receiveRequest( struct pollfd *client );
		void		_sendResponse( struct pollfd *client, unsigned int i );
		std::string	_responseBuilder();

		bool		_fdIsServer( int fdToFind );

	public:
		WebServ();
		~WebServ();
		
		void	serverRun();
		void	serverShutdown();
};

