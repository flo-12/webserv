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


class Socket
{
	//friend class WebServ;

	protected:
		int				_fd;
		SocketType		_type;

	public:
		Socket( SocketType type );
		virtual ~Socket();

		int			getFd() const;
		SocketType	getType() const;

		virtual void	setupSocket() = 0;

		bool	operator==( const Socket& other ) const;
};
