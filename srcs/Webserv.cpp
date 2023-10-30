/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/23 18:06:16 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/23 18:06:18 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"


/* DELETE AFETR CONFIG PARSER INTEGRATION */
std::vector<t_ipPort>	initializeIpPortsConfig()
{
    std::vector<t_ipPort> ipPorts;
	t_ipPort	tmp;
	tmp.port = 18000; tmp.ip = 2130706433; ipPorts.push_back(tmp);
    tmp.port = 19000; tmp.ip = 2130706434; ipPorts.push_back(tmp);
	tmp.port = 20000; tmp.ip = 2130706433; ipPorts.push_back(tmp);   
    return ipPorts;
}



/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

WebServ::WebServ()
{
	std::vector<t_ipPort> ipPorts = initializeIpPortsConfig();

	// Check for errors in Config Parser output
	if ( ipPorts.empty() )
		throw std::runtime_error("Error: no ip:port specified in config file");
	else if ( ipPorts.size() > MAX_CONNECTIONS )
		throw std::runtime_error("Error: too many server sockets specified in config file");

	// Initialize _pollFds
	for ( int i = 0; i < MAX_CONNECTIONS; ++i )
		_initPollFd( -1, 0, 0, &_pollFds[i] );

	// Create all server sockets
	for ( int i = 0; i < static_cast<int>(ipPorts.size()); ++i )
		_sockets.push_back( _createServerSocket(ipPorts[i].ip, ipPorts[i].port, i) );

	std::cout << "server initialized & listening on port" << std::endl;
}

WebServ::~WebServ()
{
	for (std::vector<Socket>::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		close(it->_fd);
}

/**************************************************************/
/*                PRIVATE METHODS (SETUP/INIT)                */
/**************************************************************/

/* _setupServer:
*	Create a new Socket for a server:
*		- create socket
*		- set socket options (reusable port and socket & non-blocking)
*		- bind socket to port
*		- listen on socket
*		- add socket fd to _pollFds
*	Return: the created socket.
*/
Socket	WebServ::_createServerSocket( unsigned int ip, int port, int pollFdIndex )
{
	Socket	serverSocket( SERVER );

	// Save initial values
	serverSocket._ip = ip;
	serverSocket._port = port;

	// Create socket
	if ( (serverSocket._fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		throw std::runtime_error("Error: createServerSocket (socket() failed)");

	// Reusable Port and Socket
	int on = 1;
	if ( setsockopt(serverSocket._fd, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT,
				&on, sizeof(int)) < 0 )
		throw std::runtime_error("Error: createServerSocket (setsockopt() failed)");
	
	// Set socket to non-blocking
	if( fcntl(serverSocket._fd, F_SETFL, O_NONBLOCK) < 0 )
		throw std::runtime_error("Error: createServerSocket (fcntl() failed)");
	
	// Identify a socket (assigning transport address to socket)
	struct sockaddr_in	server_addr;

	memset( &server_addr, 0, sizeof(server_addr) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(ip);
	server_addr.sin_port = htons(port);
	if ( bind(serverSocket._fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
		throw std::runtime_error("Error: setup server (bind() failed)");

	// Listen for connections on a socket (mark socket as passive)
	if( listen(serverSocket._fd, MAX_CONNECTIONS / 2) < 0 )
		throw std::runtime_error("Error: setup server (listen() failed)");

	// Add socket fd to _pollFds
	_initPollFd( serverSocket._fd, POLLIN, 0, &_pollFds[pollFdIndex] );

	return serverSocket;
}

/* _initPollFd:
*	Initialize a pollfd struct with the given parameters.
*/
void	WebServ::_initPollFd( int fd, short events, short revents, struct pollfd *pollFd )
{
	pollFd->fd = fd;
	pollFd->events = events;
	pollFd->revents = revents;
}


/**************************************************************/
/*               PRIVATE METHODS (ERROR/UTILS)                */
/**************************************************************/

/* _restartServerSocket:
*	Restart server socket with the given socket.
*		- close socket
*		- create new socket and add to _sockets
*		- add new socket to _pollFds
*/
void	WebServ::_restartServerSocket( Socket &socket )
{
	if ( socket._fd > 0 )
		close( socket._fd );
	std::vector<Socket>::iterator it = std::find(_sockets.begin(), _sockets.end(), socket);
	int	i = _getIndexPollFd(socket._fd);
	if ( i != -1 )
		*it = _createServerSocket( socket._ip, socket._port, _getIndexPollFd(socket._fd) );
	else
		throw std::runtime_error("Error: _restartServerSocket() failed");
}

/* _closeConnection:
*	Close client socket and remove from _pollFds and _sockets.
*/
void	WebServ::_closeConnection( Socket &socket )
{
	// Remove from _pollFds
	int	i = _getIndexPollFd(  socket._fd );
	if ( i == -1 )	// socket not found -> skip shift of _pollFds (for-loop)
		i = _sockets.size() - 1;
	for ( ; i < static_cast<int>(_sockets.size()) - 1; ++i )
		_pollFds[i] = _pollFds[i + 1];
	_pollFds[_sockets.size()].fd = -1;

	// close fd
	if ( socket._fd > 0 ) {
		close( socket._fd );
		socket._fd = -1;
	}
	
	// Remove from _sockets
	_sockets.erase( std::find(_sockets.begin(), _sockets.end(), socket) );
}

/* _getIndexPollFd:
*	Return the index _pollFds with the given fd.
*/
int		WebServ::_getIndexPollFd( int fd )
{
	for ( int i = 0; i < MAX_CONNECTIONS; ++i )
	{
		if ( _sockets[i]._fd == fd )
			return i;
	}
	return -1;
}

/* __getFreePollFd:
*	Return the index of the first free _pollFds.
*/
int	WebServ::_getFreePollFd()
{
	return _getIndexPollFd( -1 );
}


/**************************************************************/
/*               PRIVATE METHODS (SERVER LOOP)                */
/**************************************************************/

/* _pollError:
*	Check for errors on revents and
*		- restart server socket if error
*		- close client socket if error
*	Return: true if error, false otherwise.
*/
bool	WebServ::_pollError( short revent, Socket &socket)
{
	bool	pollError = false;

	if ( socket._type == SERVER)
	{
		/* if ( revent & POLLNVAL ) {		// invalid request
			std::cerr << "Error: POLLNVAL on poll() revent - restarting server socket" << std::endl;
			pollError = true;
		}
		else */ if ( revent & POLLERR ) {	// error condition
			std::cerr << "Error: POLLERR on poll() revent - restarting server socket" << std::endl;
			pollError = true;
		}
		else if ( revent & POLLHUP ) {	// hang up
			std::cerr << "Error: POLLHUB on poll() revent - restarting server socket" << std::endl;
			pollError = true;
		}
		if ( pollError )	// restart server socket
			_restartServerSocket( socket );
	}
	else if ( socket._type == CLIENT )
	{
		if ( revent & POLLNVAL ) {		// invalid request
			std::cerr << "Error: POLLNVAL on poll() revent - closing client socket" << std::endl;
			pollError = true;
		}
		else if ( revent & POLLERR ) {	// error condition
			std::cerr << "Error: POLLERR on poll() revent - closing client socket" << std::endl;
			pollError = true;
		}
		else if ( revent & POLLHUP ) {	// hang up
			std::cerr << "Error: POLLHUB on poll() revent - closing client socket" << std::endl;
			pollError = true;
		}
		if ( pollError ) {	// close client socket & remove from _pollFds and _sockets
			_closeConnection( socket );
			// Send 500 Internal Server Error?????
		}
	}

	return pollError;
}

/* _acceptNewConnection:
*	Accept new connection and add client to _pollFds and _sockets.
*	Nothing happens if MAX_CONNECTIONS is reached or no free _pollFds.
*/
void	WebServ::_acceptNewConnection( Socket &serverSocket )
{
	if ( _sockets.size() >= MAX_CONNECTIONS )	// MAX_CONNECTIONS reached
	{
		std::cerr << "Error: _acceptNewConnection (MAX_CONNECTIONS reached)." << std::endl;
		return ;
	}
	int	indexFreeFd = _getFreePollFd();
	if ( indexFreeFd == -1 )	// no free _pollFds
	{
		std::cerr << "Error: _acceptNewConnection (no free _pollFds)." << std::endl;
		return ;
	}

	Socket	clientSocket( CLIENT );

	clientSocket._fd = accept(serverSocket._fd, NULL, NULL);
	if ( clientSocket._fd < 0 )
	{
		std::cerr << "Error: _acceptNewConnection (accept() failed)." << std::endl;
		return ;
	}

	if ( fcntl(clientSocket._fd, F_SETFL, O_NONBLOCK) < 0 )	// set socket to non-blocking
	{
		std::cerr << "Error: _acceptNewConnection (fcntl() failed)." << std::endl;
		return ;
	}

	_sockets.push_back( clientSocket );

	_initPollFd( clientSocket._fd, POLLIN, 0, &_pollFds[indexFreeFd] );
}

/* _receiveRequest (1st version):
*	Receive request from client and store in internal socket.t_request.buffer.
*	1st version contains:
*		- reading GET request only.
*		- check for recv() errors - return is -1
*		- assuming request is complete (no chunked transfer encoding).

*	Further versions:
*		- handle POST request (content-length, chunked transfer encoding)
*		- handle request bigger than buffer
*		- handle errno of recv() (EAGAIN, EWOULDBLOCK, EINTR)
*/
void	WebServ::_receiveRequest( Socket &clientSocket )
{
	int		indexPollFd = _getIndexPollFd( clientSocket._fd );
	char	buffer[MAX_REQ_SIZE] = {0};
	ssize_t	bytesRead;

	// Receive request
	if ( (bytesRead = recv(clientSocket._fd, &buffer, MAX_REQ_SIZE, O_NONBLOCK)) < 0 
		|| indexPollFd == -1 )
	{
		// send 500 Internal Server Error?!
		_closeConnection( clientSocket );
		return ;
	}

	// Check if request is complete
	size_t	found = std::string(buffer).find("\r\n\r\n");
	if (found != std::string::npos)
	{
		std::cout << "Request received complete" << std::endl;	
		_initPollFd( clientSocket._fd, POLLOUT, 0, &_pollFds[indexPollFd] );
		clientSocket._request.pendingReceive = false;
		//clientSocket._request.contentLength = 0;
		clientSocket._request.readBytes += bytesRead;
		clientSocket._request.buffer += buffer;

		std::cout << "+++++++++++++++ Request +++++++++++++++" << std::endl;
		std::cout << clientSocket._request.buffer << std::endl;
		std::cout << "++++++++++++++++++++++++++++++++++++++++" << std::endl;
	}
	else
		std::cout << "Request received incomplete (buffer not big enough)" << std::endl;
}

/* _sendResponse (1st version):
*	Send response to client.
	- delete request from socket?
	- store response in socket?
*	- respond index.html or "Hello World"?!
*/
void	WebServ::_sendResponse( Socket &clientSocket )
{
	clientSocket.buildResponse();
	int		indexPollFd = _getIndexPollFd( clientSocket._fd );

	std::string	msgSend = clientSocket._response.header + clientSocket._response.body;
	ssize_t	bytesSent = send(clientSocket._fd, msgSend.c_str(), msgSend.length(), 0);
	if ( bytesSent < static_cast<ssize_t>(-1) || indexPollFd == -1 ) {
		// send 500 Internal Server Error?! -> clientSocket.buildResponse(500) -> clientSocket.keepAlive = false -> _sendResponse(clientSocket)
		_closeConnection( clientSocket );
		return ;
	}
	clientSocket._response.sentBytes += bytesSent;

	if ( clientSocket._response.sentBytes == clientSocket._response.contentLength )
	{
		//_closeConnection( clientSocket );
		clientSocket._response.pendingSend = false;
		_initPollFd( clientSocket._fd, POLLIN, 0, &_pollFds[indexPollFd] );
	}
	else
	{
		//_initPollFd( clientSocket._fd, POLLOUT, 0, &_pollFds[indexPollFd] );
		clientSocket._response.pendingSend = true;
	}
}


/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

/* serverRun:
*	Server routine (infinite loop) for receiving and sending data.
*		- poll for events and check for errors
*		- accept new connections
*		- read requests
*		- send responses
*		- check for client timeout
*/
void	WebServ::serverRun()
{
	int		retPoll;
	bool	pollError = false;
	int		indexPollFd;

	while ( 42 ) {
		// Poll for events and check for errors
		if ( (retPoll = poll(_pollFds, _sockets.size(), TIMEOUT_POLL)) < 0 )
			throw std::runtime_error("Error: poll() failed");
		
		for ( int i = 0; i < static_cast<int>(_sockets.size()); ++i )
		{
			if ( (indexPollFd = _getIndexPollFd( _sockets[i]._fd )) == -1 )
			{
				if ( _sockets[i]._type == SERVER ) {
					std::cerr << "Error: serverRun() - server socket not found in _pollFds -> restart server" << std::endl;
					_restartServerSocket( _sockets[i] );
				}
				else if ( _sockets[i]._type == CLIENT ) {
					std::cerr << "Error: serverRun() - client socket not found in _pollFds -> close connection" << std::endl;
					_closeConnection( _sockets[i] );
				}
				continue ;
			}				
			// Check for errors on revents and delete client socket if error
			if ( (pollError = _pollError( _pollFds[indexPollFd].revents, _sockets[i] )) ) {
				;
			}
			// Accept new connections
			else if ( _pollFds[indexPollFd].revents & POLLIN )
				_acceptNewConnection( _sockets[i] );
			// Read requests
			else if ( _pollFds[indexPollFd].revents & POLLIN )
				_receiveRequest( _sockets[i] );
			// Send responses
			else if ( _pollFds[indexPollFd].revents & POLLOUT )
				_sendResponse( _sockets[i] );
			// Check for client timeout
			/* else if ( !pollError && _sockets[i]._type == CLIENT )
				_checkTimeout( _sockets[i] ); */
			pollError = false;
		}
	}
}

