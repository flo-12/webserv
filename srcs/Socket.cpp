/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/27 16:07:06 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/27 16:07:10 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"


/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

Socket::Socket( SocketType type, ServerConfig config )
	: _type(type), _config(config)
{
}

Socket::~Socket()
{
	if ( _fd != -1 )
		close(_fd);
}


/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

int			Socket::getFd() const
{
	return _fd;
}

SocketType	Socket::getType() const
{
	return _type;
}

ServerConfig	Socket::getConfig() const
{
	return _config;
}


/**************************************************************/
/*                   OVERLOAD OPERATORS                       */
/**************************************************************/

bool	Socket::operator==( const Socket& other ) const
{
	return _fd == other._fd;
}