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

	// Initialize _pollFds with -1
	for ( int i = 0; i < MAX_CONNECTIONS; ++i )
		_initPollFd( -1, 0, 0, &_pollFds[i] );

	// Create all server sockets and add fd to _pollFds
	for ( int i = 0; i < static_cast<int>(ipPorts.size()); ++i ) {
		ServerSocket*	serverSocket = new ServerSocket( ipPorts[i].ip, ipPorts[i].port );
		_sockets.push_back( serverSocket );

		_initPollFd( serverSocket->getFd(), POLLIN, 0, &_pollFds[i] );
	}

	std::cout << "server initialized & listening on port" << std::endl;
}

WebServ::~WebServ()
{
	std::cout << "WebServ destructor" << std::endl;
	serverShutdown();
}


/**************************************************************/
/*                PRIVATE METHODS (SETUP/INIT)                */
/**************************************************************/

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
*	Restart server socket and update _pollFds.
*/
void	WebServ::_restartServerSocket( ServerSocket *socket )
{
	int	i = _getIndexPollFd( socket->getFd() );
	socket->restartServerSocket();
	if ( i != -1 )
		_initPollFd( socket->getFd(), POLLIN, 0, &_pollFds[i] );
	else
	{
		i = _getFreePollFd();
		if ( i == -1 ) {
			std::cerr << "Error: _restartServerSocket (no free _pollFds) - taking last possible." << std::endl;
			i = MAX_CONNECTIONS - 1;
			close( _pollFds[i].fd );
		}
		_initPollFd( socket->getFd(), POLLIN, 0, &_pollFds[_getFreePollFd()] );
	}
}

/* _forgetConnection:
*	Remove socket->_fd from _pollFds, eras from _sockets and delete instance.
*/
void	WebServ::_forgetConnection( Socket *socket, HttpErrorType httpError )
{
	std::cout << "Socket forgetting (fd: " << socket->getFd() << ")" << std::endl;

	// Send error page if client socket
	if ( socket->getType() == CLIENT )
		dynamic_cast<ClientSocket*>(socket)->closeConnection(httpError);

	// Remove from _pollFds
	int	i = _getIndexPollFd(  socket->getFd() );
	if ( i != -1 )
	{
		for ( ; i < static_cast<int>(_sockets.size()) - 1; ++i )
			_pollFds[i] = _pollFds[i + 1];
		_pollFds[_sockets.size()].fd = -1;
	}

	// Remove from _sockets and delete socket
	_sockets.erase( std::find(_sockets.begin(), _sockets.end(), socket) );
	delete socket;
}

/* _getIndexPollFd:
*	Return the index _pollFds with the given fd.
*/
int		WebServ::_getIndexPollFd( int fd )
{
	for ( int i = 0; i < MAX_CONNECTIONS; ++i )
	{
		if ( _pollFds[i].fd == fd )
			return i;
	}
	return -1;
}

/* _getHandleIdxPollFd:
*	Return the index _pollFds with the given fd.
*	In case that fd is not found:
*		- Return -1
*		- closeConnection if client socket
*		- restartServerSocket if server socket
*/
int	WebServ::_getHandleIdxPollFd( Socket *socket )
{
	int indexPollFd = _getIndexPollFd( socket->getFd() );
	
	if ( indexPollFd != -1 )
		return indexPollFd;
	else
	{
		if ( socket->getType() == SERVER ) {
			std::cerr << "Error: serverRun() - server socket not found in _pollFds -> restart server" << std::endl;
			dynamic_cast<ServerSocket*>(socket)->restartServerSocket();
		}
		else if ( socket->getType() == CLIENT ) {
			std::cerr << "Error: serverRun() - client socket not found in _pollFds -> close connection" << std::endl;
			_forgetConnection( socket, ERROR_500 );
		}
		return -1;
	}
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
bool	WebServ::_pollError( short revent, Socket *socket)
{
	bool	pollError = false;

	if ( socket->getType() == SERVER)
	{
		if ( revent & POLLNVAL ) {		// invalid request
			std::cerr << "Error: POLLNVAL on poll() revent - restarting server socket (fd: " << socket->getFd() << ")" << std::endl;
			pollError = true;
		}
		else if ( revent & POLLERR ) {	// error condition
			std::cerr << "Error: POLLERR on poll() revent - restarting server socket (fd: " << socket->getFd() << ")" << std::endl;
			pollError = true;
		}
		else if ( revent & POLLHUP ) {	// hang up
			std::cerr << "Error: POLLHUB on poll() revent - restarting server socket (fd: " << socket->getFd() << ")" << std::endl;
			pollError = true;
		}
		if ( pollError )	// restart server socket
			_restartServerSocket( dynamic_cast<ServerSocket*>(socket) );
	}
	else if ( socket->getType() == CLIENT )
	{
		if ( revent & POLLNVAL ) {		// invalid request
			std::cerr << "Error: POLLNVAL on poll() revent - closing client socket socket (fd: " << socket->getFd() << ")" << std::endl;
			pollError = true;
		}
		else if ( revent & POLLERR ) {	// error condition
			std::cerr << "Error: POLLERR on poll() revent - closing client socket (fd: " << socket->getFd() << ")" << std::endl;
			pollError = true;
		}
		else if ( revent & POLLHUP ) {	// hang up
			std::cerr << "Error: POLLHUB on poll() revent - closing client socket (fd: " << socket->getFd() << ")" << std::endl;
			pollError = true;
		}
		if ( pollError )	// close client socket & remove from _pollFds and _sockets
			_forgetConnection( socket, ERROR_500 );
	}

	return pollError;
}

/* _acceptNewConnection:eleted
*	Accept new connection and add client to _pollFds and _sockets.
*	Nothing happens if MAX_CONNECTIONS is reached or no free _pollFds.
*/
void	WebServ::_acceptNewConnection( ServerSocket *serverSocket )
{
	std::cout << "Accepting new connection" << std::endl;

	int	indexFreeFd = _getFreePollFd();
	if ( indexFreeFd == -1 )	// no free _pollFds
	{
		std::cerr << "Error: _acceptNewConnection (no free _pollFds)." << std::endl;
		return ;
	}

	Socket	*clientSocket = new ClientSocket( serverSocket->getFd() );
	try
	{
		clientSocket->setupSocket();
		_sockets.push_back( clientSocket );
		_initPollFd( clientSocket->getFd(), POLLIN, 0, &_pollFds[indexFreeFd] );
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		delete clientSocket;
	}	
}

/* _receiveRequest (1st version):
*	Receive request at client. Client handles the receiving and storing the request
*	and returns the status of the receiving process.
*/
void	WebServ::_receiveRequest( ClientSocket *clientSocket )
{
	std::cout << "Receiving request" << std::endl;

	int		indexPollFd = _getHandleIdxPollFd( clientSocket );
	if ( indexPollFd == -1 )
		return ;

	ReceiveStatus	receiveStatus = clientSocket->receiveRequest();
	if ( receiveStatus == READ_ERROR )
		_forgetConnection( clientSocket, ERROR_400 );
	else if ( receiveStatus == CLIENT_CLOSED )
		_forgetConnection( clientSocket, NO_ERROR );
	else if ( receiveStatus == READ_DONE )
		_initPollFd( clientSocket->getFd(), POLLOUT, 0, &_pollFds[indexPollFd] );
}

/* _sendResponse (1st version):
*	Send response to client. Client handles the sending of the response and 
*	returns the status of the sending process.
*	Close connection if error. Set _pollFds to POLLIN in case of success.
*/
void	WebServ::_sendResponse( ClientSocket *clientSocket )
{
	std::cout << "Sending response" << std::endl;

	int		indexPollFd = _getHandleIdxPollFd( clientSocket );
	if ( indexPollFd == -1 )
		return ;

	ResponseStatus	responseStatus = clientSocket->sendResponse();
	if ( responseStatus == SEND_ERROR )
		_forgetConnection( clientSocket, ERROR_400 );
	else if ( responseStatus == SEND_DONE )
	{
		_initPollFd( clientSocket->getFd(), POLLIN, 0, &_pollFds[indexPollFd] );
		std::cout << "Sending Response done" << std::endl;
	}	
}


/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

/* serverRun:
*	Server routine (infinite loop) for receiving and sending data.
*	
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
			if ( (indexPollFd = _getHandleIdxPollFd( _sockets[i] )) == -1 )
				continue ;
			
			// Check for errors on revents and delete client socket if error
			if ( (pollError = _pollError( _pollFds[indexPollFd].revents, _sockets[i]) ) )
				;
			// Accept new connections
			else if ( _pollFds[indexPollFd].revents & POLLIN && _sockets[i]->getType() == SERVER )
				_acceptNewConnection( dynamic_cast<ServerSocket*>(_sockets[i]) );
			// Read requests
			else if ( _pollFds[indexPollFd].revents & POLLIN && _sockets[i]->getType() == CLIENT )
				_receiveRequest( dynamic_cast<ClientSocket*>(_sockets[i]) );
			// Send responses
			else if ( _pollFds[indexPollFd].revents & POLLOUT && _sockets[i]->getType() == CLIENT )
				_sendResponse( dynamic_cast<ClientSocket*>(_sockets[i]) );
			// Check for client timeout (if no request received within XXXs)
			// else if ( !pollError && _sockets[i]->getType() == CLIENT )
			//	_checkTimeout( dynamic_cast<ClientSocket*>(_sockets[i]) );
			pollError = false;
		}
	}
}

/* serverShutdown:
*	Shutdown server and close all sockets by deleting them.
*/
void	WebServ::serverShutdown()
{
	for (std::vector<Socket*>::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		delete (*it);
}
