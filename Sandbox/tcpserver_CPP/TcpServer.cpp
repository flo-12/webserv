#include "TcpServer.hpp"


/**************************************************************/
/*                      CANONICAL FORM                        */
/**************************************************************/

TcpServer::TcpServer()
	: _serverPort(SERVER_PORT), _serverIp(IP_ADDRESS), _maxConnections(MAX_CONNECTIONS), 
	_maxGetSize(MAX_GET_SIZE), _serverFd(-1)
{
	_setupServer();
	std::cout << "server initialized & listening on port: " << _serverPort << std::endl;
	_initFdSet();
}

TcpServer::~TcpServer()
{
	if ( _serverFd != -1 )
		close(_serverFd);
	/* if (_client_fd != -1)
		close(_client_fd); */
}

/**************************************************************/
/*                     PRIVATE METHODS                        */
/**************************************************************/

void	TcpServer::_setupServer()
{
	// Create socket
	if ( (_serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		std::cout << "Error: setup server (socket() failed)" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Reusable Port and Socket
	int on = 1;
	if ( setsockopt(_serverFd, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT,
				&on, sizeof(int)) < 0 )
	{
		std::cout << "Error: setup server (setsockopt() failed)" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Identify a socket (assigning transport address to socket)
	struct sockaddr_in	server_addr;

	memset( &server_addr, 0, sizeof(server_addr) );
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(_serverIp);
	server_addr.sin_port = htons(_serverPort);
	if ( bind(_serverFd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
	{
		std::cout << "Error: setup server (bind() failed)" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Listen for connections on a socket (mark socket as passive)
	if( listen(_serverFd, _maxConnections) < 0 )
    {
		std::cout << "Error: setup server (listen() failed)" << std::endl;
		return ;
	}
}

void	TcpServer::_initFdSet()
{
	FD_ZERO(&_fdSet);
	FD_SET(_serverFd, &_fdSet);
}


int	TcpServer::_acceptNewConnection( int server_fd )
{
	int					addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in	client_addr;
    int					clientFd;

	clientFd = accept(server_fd, 
			(struct sockaddr *)&client_addr, 
			(socklen_t *)&addr_size);
	return clientFd;
}

void*	TcpServer::_handleConnection( int client_socket )
{
	char	buffer[_maxGetSize];
	size_t	bytes_read;
	int		msgsize = 0;
	//int		file;
	//char		actualpath[PATH_MAX + 1];

	while ((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer) - msgsize - 1)) > 0)
    {
		msgsize += bytes_read;
		if (msgsize > _maxGetSize - 1 || buffer[msgsize-1] == '\n')
			break;
	}
	buffer[msgsize - 1] = 0;
	std::cout << buffer << std::endl;
	fflush( stdout );

	snprintf((char *)buffer, sizeof(buffer), "HTTP/1.0 200 OK\r\n\r\nHello World");
	write(client_socket, (char *)buffer, strlen((char *)buffer));
	// file = open("index.html", O_RDONLY);
	// if (file < 0)
	// {
	//		printf("could not open file\n");
	//		exit(1);
	// }
	// while ( (bytes_read = read(file, buffer, _maxGetSize - 1)) > 0)
	//		write (client_socket, buffer, bytes_read);
	close( client_socket );
	// close(file);
	return NULL;
}



/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

void	TcpServer::serverRun()
{
	std::cout << "server running and responding..." << std::endl;

	int		client_socket;
	fd_set	ready_sockets;
	//char	*request;

	while (42)
	{
		ready_sockets = _fdSet;

		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
		{
			printf("Error, select failed\n");
			exit (1);
		}

		for (int i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &ready_sockets))
			{
				// two cases:
				// 1. new connection that we want to accept
				if (i == _serverFd)
				{
					client_socket = this->_acceptNewConnection( _serverFd );
					FD_SET(client_socket, &_fdSet);
				}
				else // 2. read data from an existing connection
				{
					// request = server.request_parser(client_socket);
					// server.response_builder(client_socket, request);
					this->_handleConnection( client_socket );
					FD_CLR(i, &_fdSet);
				}
			}
		}
	}
}


/**************************************************************/
/*                 NOT USED PUBLIC METHODS                    */
/**************************************************************/

void	TcpServer::responseBuilder( int client_socket, char *buffer )
{
    int     file;
    size_t  bytes_read;
    
    file = open("index.html", O_RDONLY);
    if (file < 0)
    {
        printf("could not open file\n");
        exit(1);
    }
    while ( (bytes_read = read(file, buffer, _maxGetSize - 1)) > 0 )
        write (client_socket, buffer, bytes_read);
    close(client_socket);
    close(file);
}

char*	TcpServer::requestParser( int client_socket )
{
    char    buffer[_maxGetSize];
    size_t  bytes_read;
    int     msgsize = 0;
    // char    actualpath[PATH_MAX + 1];

    while ((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer) - msgsize - 1)) > 0)
    {
            msgsize += bytes_read;
            if (msgsize > _maxGetSize - 1 || buffer[msgsize-1] == '\n')
                break;
    }
    buffer[msgsize - 1] = 0;
    printf("%s\n", buffer);
    fflush(stdout);
    return (NULL);
}
