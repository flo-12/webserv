#pragma once

#include <iostream>
#include <cstdlib> // for exit()
#include <sys/socket.h> // for socket()
#include <netdb.h> // for struct sockaddr_in
#include <string.h> // for bzero(), strlen()
#include <unistd.h> // for close(), read(), write()
#include <stdio.h> // for printf(), snprintf(), fflush()
#include <fcntl.h> // for open(), O_RDONLY


# define IP_ADDRESS 2130706433	/* 127.0.0.1 converted to int */
# define SERVER_PORT 18000
# define BUFSIZE 8192


/* Class to run a TCP-Server
*	- default constructor initializes the server attributes
*	- setup_server() creates a socket and binds it to a port
*
*/
class TcpServer
{
	private:
/* 		int					_server_socket;
		struct sockaddr_in	_server_addr;
		int					_client_socket;
		int					_addr_size;
		struct sockaddr_in	_client_addr; */
		int	_serverPort;
		int	_serverFd;

		void	_setupServer();
		int		_acceptNewConnection( int server_fd );

	public:
		TcpServer();
		~TcpServer();
		
		void	serverRun();
		
		void*	_handleConnection( int client_socket );
		
		void	responseBuilder( int client_socket, char *buffer );
		char*	requestParser( int client_socket );
		
		

		/* int accept_new_connection(int server_socket);
		int get_server_socket();
		int get_client_socket();
		struct sockaddr_in get_client_addr();
		int get_addr_size();
		void set_client_socket(int client_socket);
		void set_client_addr(struct sockaddr_in client_addr);
		void set_addr_size(int addr_size);
		void close_server_socket();
		void close_client_socket(); */
};
