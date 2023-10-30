/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/23 18:06:08 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/23 18:06:11 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sys/socket.h> // for socket()
#include <netdb.h> // for struct sockaddr_in
#include <string> // for std::string
#include <string.h> // for bzero(), strlen()
#include <unistd.h> // for close(), read(), write()
#include <stdio.h> // for printf(), snprintf(), fflush()
#include <fcntl.h> // for open(), O_RDONLY
#include <cerrno> // for errno
#include <poll.h> // for poll
#include <exception> // for exception
#include <vector> // for vector
#include <algorithm>

#include "Socket.hpp"



/************ INTERFACE TO CONFIG_PARSER ************/
# define MAX_REQ_SIZE 8192
# define MAX_CONNECTIONS 50
# define TIMEOUT_POLL 5000

typedef struct s_ipPort
{
	int				port;
	unsigned int	ip;
} t_ipPort;


/************ START CLASS ************/

class WebServ
{
	private:
		struct pollfd			_pollFds[MAX_CONNECTIONS];
		std::vector<Socket*>	_sockets;

		// Setup / Init
		Socket&	_createServerSocket( unsigned int ip, int port, int pollFdIndex );
		void	_initPollFd( int fd, short events, short revents, struct pollfd *pollFd );

		// Server Loop
		bool	_pollError( short revent, Socket *socket);
		void	_acceptNewConnection( Socket *serverSocket );
		void	_receiveRequest( Socket *clientSocket );
		void	_sendResponse( Socket *clientSocket );

		// Error / Utils
		void	_closeConnection( Socket *socket );
		void	_restartServerSocket( Socket *socket );
		int		_getIndexPollFd( int fd );
		int		_getFreePollFd();

		
	public:
		WebServ();
		~WebServ();
		
		void	serverRun();
		void	serverShutdown();
};

