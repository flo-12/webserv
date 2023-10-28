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

Socket::Socket( int fd )
{
	_fd = fd;
	_type = UNUSED;
	_reqStatus.pendingReceive = false;
	_reqStatus.contentLength = 0;
	_reqStatus.readBytes = 0;
}

Socket::~Socket( int fd )
{
	if ( _fd != -1 )
		close( _fd );
}