#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

# define SERVER_PORT 18000

int main(int argc, char**argv)
{
    int                 listenfd, connfd, n;
    struct sockaddr_in  servaddr;
    uint8_t             buff[4000];
    uint8_t             recvline[4000];

    // creating a socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        printf("Error: could not create socket\n");
        return (0);
    }

    // setting up the address
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    //binding socket to address
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        printf("Error: could not bind\n");
        return (0);
    }
    if(listen(listenfd, 10) < 0)
    {
        printf("Error: could not listen\n");
        return (0);
    }

    for(;;)
    {
        struct sockaddr_in  addr;
        socklen_t           addr_lem;

        // wait for a request and print it to terminal
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
        memset(recvline, 0, 4000);
        while( (n = read(connfd, recvline, 3999)) > 0 );
        {
            printf("\n%s\n", recvline);
            if (recvline[n-1] = '\n')
                break;
        }
        memset(recvline, 0, 4000);
        if(n < 0)
        {
            printf("Error: read error\n");
            return (0);
        }

        // send a response
        snprintf((char *)buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHello");
        write(connfd, (char *)buff, strlen((char *)buff));
        close(connfd);
    }
}