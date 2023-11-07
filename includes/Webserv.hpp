/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/23 18:06:08 by fbecht            #+#    #+#             */
/*   Updated: 2023/11/01 07:49:51 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP



/************ INCLUDES ************/
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

#include "common.hpp"
#include "ServerSocket.hpp"
#include "ClientSocket.hpp"


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
		bool	_pollError( short revent, Socket *socket );
		void	_acceptNewConnection( ServerSocket *serverSocket );
		void	_receiveRequest( ClientSocket *clientSocket );
		void	_sendResponse( ClientSocket *clientSocket );

		// Error / Utils
		void	_forgetConnection( Socket *socket, HttpStatusCode httpStatus );
		void	_restartServerSocket( ServerSocket *socket );
		int		_getHandleIdxPollFd( Socket *socket );
		int		_getIndexPollFd( int fd );
		int		_getFreePollFd();
		
	public:
		WebServ( std::vector<t_ipPort> configInfo, std::vector<ServerConfig> _serverConfigs );
		~WebServ();
		
		void	serverRun();
		void	serverShutdown();
};

#endif