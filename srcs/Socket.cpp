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
	std::cout << "Socket destructor (ip: " << _ip << ", port: " << _port << ")" << std::endl;
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
	_response.body = _readFile("./html/index.html");
	std::stringstream	strStream;
	strStream << (_response.body.length());

	_response.header = "HTTP/1.1 200 OK\r\n";
	_response.header += "Content-Type: text/html\r\n";
	_response.header += "Content-Length: " + strStream.str() + "\r\n";
	_response.header += "\r\n";
	
	//_response.body = "Hello World\n";

	_response.contentLength = _response.header.length() + _response.body.length();
}

std::string	Socket::_readFile( std::string file )
{
	std::cout << "Reading file: " << file << std::endl;
	std::string		line;
	std::string		body;
	std::ifstream	ifs(file.c_str());

	if ( !ifs.is_open() )
		throw std::runtime_error("Error: readFile() failed");

	while ( getline( ifs, line ) )
		body += line + "\n";
	ifs.close();

	return body;
}


/* _setupServer:
*	Create a new Socket for a server:
*		- create socket
*		- set socket options (reusable port and socket & non-blocking)
*		- bind socket to port
*		- listen on socket
*		- add socket fd to _pollFds
*	Return: the created socket.
*/
void	Socket::_createServerSocket( unsigned int ip, int port )
{
	//Socket	serverSocket( SERVER );

	// Save initial values
	_ip = ip;
	_port = port;

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
	server_addr.sin_addr.s_addr = htonl(ip);
	server_addr.sin_port = htons(port);
	if ( bind(_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
		throw std::runtime_error("Error: setup server (bind() failed)");

	// Listen for connections on a socket (mark socket as passive)
	if( listen(_fd, MAX_CONNECTIONS) < 0 )
		throw std::runtime_error("Error: setup server (listen() failed)");

}



/**************************************************************/
/*                   OVERLOAD OPERATORS                       */
/**************************************************************/

bool	Socket::operator==( const Socket& other ) const
{
	return _fd == other._fd;
}