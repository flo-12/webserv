/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/27 16:07:37 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/27 16:07:39 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <unistd.h>
#include <sys/socket.h> // for socket()
#include <netdb.h> // for struct sockaddr_in
#include <exception> // for exception
#include <cstring> // for memset()
#include <fcntl.h> // for fcntl()
#include <fstream> // for ifstream
#include <sstream> // for stringstream

#include "common.hpp"
#include "ServerConfig.hpp"

//# define MAX_CONNECTIONS 500

/* #ifdef MAX_CONNECTIONS
#pragma message("MAX_CONNECTIONS is defined")
#else
#pragma message("MAX_CONNECTIONS is not defined")
#endif */

typedef enum eSocketType
{
	UNUSED,
	SERVER,
	CLIENT
} SocketType;


class Socket
{
	protected:
		int				_fd;
		SocketType		_type;
		ServerConfig	_config;

	public:
		Socket( SocketType type, ServerConfig config );
		virtual ~Socket();

		int				getFd() const;
		SocketType		getType() const;
		ServerConfig	getConfig() const;

		virtual void	setupSocket() = 0;

		bool	operator==( const Socket& other ) const;
};

#endif