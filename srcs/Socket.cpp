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

Socket::Socket( SocketType type )
	: _type(type)
{
	_fd = -1;
	_ip = 0;
	_port = 0;
	_request.pendingReceive = false;
	_request.contentLength = 0;
	_request.readBytes = 0;
	_response.pendingSend = false;
	_response.contentLength = 0;
	_response.sentBytes = 0;
}

Socket::~Socket()
{
	if ( _fd != -1 )
		close(_fd);
}


/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

/* buildResponse():
*	Built the response corresponding to the _request and stores it in _response.
*	1st draft contains:
*		- filling out all values from _request
*		- "Hello World" in the body
*		- status line ("HTTP/1.1 200 OK"), "Content-Length" (text/html) and "Content-Length" in the header
*/
void	Socket::buildResponse()
{
	_response.header = "HTTP/1.1 200 OK\r\n";
	_response.header += "Content-Type: text/html\r\n";
	_response.header += "Content-Length: 12\r\n";
	_response.header += "\r\n";
	_response.body = "Hello World\n";

	_response.contentLength = _response.header.length() + _response.body.length();
}


/**************************************************************/
/*                   OVERLOAD OPERATORS                       */
/**************************************************************/

bool	Socket::operator==( const Socket& other ) const
{
	return _fd == other._fd;
}