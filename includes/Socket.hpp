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

#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/socket.h> // for socket()
#include <netdb.h> // for struct sockaddr_in
#include <exception> // for exception
#include <cstring> // for memset()
#include <fcntl.h> // for fcntl()
#include <fstream> // for ifstream
#include <sstream> // for stringstream

# define MAX_CONNECTIONS 50

typedef enum eSocketType
{
	UNUSED,
	SERVER,
	CLIENT
} SocketType;

typedef struct s_reqStatus {
	bool		pendingReceive;
	ssize_t		contentLength;
	ssize_t		readBytes;
	std::string	buffer;
} t_reqStatus;

typedef struct s_resStatus {
	bool		pendingSend;
	ssize_t		contentLength;
	ssize_t		sentBytes;
	std::string	header;
	std::string	body;
} t_resStatus;

class Socket
{
	friend class WebServ;

	private:
		unsigned int	_ip;
		int				_port;
		int				_fd;
		SocketType		_type;
		t_reqStatus		_request;
		t_resStatus		_response;

		std::string	_readFile( std::string file );

	public:
		Socket( SocketType type );
		~Socket();

		void	_createServerSocket( unsigned int ip, int port );

		//void		createRequest( std::string request );
		void		buildResponse();


		bool	operator==( const Socket& other ) const;
};
