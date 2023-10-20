#include "TcpServer.hpp"


/**************************************************************/
/*                      CANONICAL FORM                        */
/**************************************************************/

TcpServer::TcpServer()
	: _serverPort(SERVER_PORT)
{
	/* _server_fd = -1;
	_port = -1;
	_backlog = -1;
	_addr_size = sizeof(struct sockaddr_in); */

	_setupServer();

}

TcpServer::~TcpServer()
{
	/* if (_server_fd != -1)
		close(_server_fd);
	if (_client_fd != -1)
		close(_client_fd); */
}

/**************************************************************/
/*                     PRIVATE METHODS                        */
/**************************************************************/

void	TcpServer::_setupServer()
{
	struct sockaddr_in	server_addr;

	// Create socket
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);	
	if (_serverFd < 0)
	{
		std::cout << "Error: socket creation" << std::endl;
		std::exit(1);
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(IP_ADDRESS);
	server_addr.sin_port = htons(_serverPort);

	// this lets us reuse an IP address or port which has been closed recently
	int on = 1;
	if (setsockopt(_serverFd, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT,
				&on, sizeof(int)) < 0)
	{
		std::cout << "Error: setsockopt() failed" << std::endl;
		close(_serverFd);
		std::exit(1);
	}

	if (bind(_serverFd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		std::cout << "Error: bind() failed" << std::endl;
		return ;
	}
	if(listen(_serverFd, 10) < 0)
    {
		std::cout << "Error:listen() failed" << std::endl;
		return ;
	}
}

int	TcpServer::_acceptNewConnection( int server_fd )
{
	int					addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in	client_addr;
    int					client_socket;

	client_socket = accept(server_fd, 
			(struct sockaddr *)&client_addr, 
			(socklen_t *)&addr_size);
	return client_socket;
}

void*	TcpServer::_handleConnection( int client_socket )
{
	char	buffer[BUFSIZE];
	size_t	bytes_read;
	int		msgsize = 0;
	//int		file;
	//char		actualpath[PATH_MAX + 1];

	while ((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer) - msgsize - 1)) > 0)
    {
		msgsize += bytes_read;
		if (msgsize > BUFSIZE - 1 || buffer[msgsize-1] == '\n')
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
	// while ( (bytes_read = read(file, buffer, BUFSIZE - 1)) > 0)
	//		write (client_socket, buffer, bytes_read);
	close(client_socket);
	// close(file);
	return NULL;
}



/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

void	TcpServer::serverRun()
{
	int		client_socket;
	fd_set	current_sockets;
	fd_set	ready_sockets;
	//char	*request;

	std::cout << "server listening on port: " << _serverPort << std::endl;

	FD_ZERO(&current_sockets);				// set the fd set to zero
	FD_SET(_serverFd, &current_sockets);	// add fd to set

	while (42)
	{
		ready_sockets = current_sockets;

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
					client_socket = this->_acceptNewConnection(_serverFd);
					FD_SET(client_socket, &current_sockets);
				}
				else // 2. read data from an existing connection
				{
					// request = server.request_parser(client_socket);
					// server.response_builder(client_socket, request);
					this->_handleConnection( client_socket );
					FD_CLR(i, &current_sockets);
				}
			}
		}
	}
}


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
    while ( (bytes_read = read(file, buffer, BUFSIZE - 1)) > 0 )
        write (client_socket, buffer, bytes_read);
    close(client_socket);
    close(file);
}

char*	TcpServer::requestParser( int client_socket )
{
    char    buffer[BUFSIZE];
    size_t  bytes_read;
    int     msgsize = 0;
    // char    actualpath[PATH_MAX + 1];

    while ((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer) - msgsize - 1)) > 0)
    {
            msgsize += bytes_read;
            if (msgsize > BUFSIZE - 1 || buffer[msgsize-1] == '\n')
                break;
    }
    buffer[msgsize - 1] = 0;
    printf("%s\n", buffer);
    fflush(stdout);
    return (NULL);
}
