#ifndef COMMON_HPP
# define COMMON_HPP

# include <vector>
# include <string>
# include <sstream>


/**************************************************************/
/*                        DEFINES                             */
/**************************************************************/

// Default server configurations:
# define DEFAULT_HOST "127.0.0.1"
# define DEFAULT_ROOT "/www"
# define DEFAULT_CLIENT_MAX_BODY_SIZE 300000
# define DEFAULT_INDEX "index.html"
# define DEFAULT_PORT 8000

// Webserv:
# define MAX_CONNECTIONS 500
# define TIMEOUT_POLL 5000
# define MAX_REQ_SIZE 8192
# define TIMEOUT_RECEIVE 8192

// Response:
# define HTTP_VERSION "HTTP/1.1"
# define HTTP_STATS_CODE_FILE "/database/http_status_codes.csv"


/**************************************************************/
/*                        STRUCTS                             */
/**************************************************************/

typedef struct	s_ipPort
{
	unsigned int	ip;
	int				port;
}	t_ipPort;

typedef enum	e_httpMethod
{
	NO_TYPE,
	GET,
	POST,
	DELETE,
	HEAD,
	PUT,
	OPTIONS,
	TRACE,
	CONNECT
}   httpMethod;

typedef enum e_HttpStatusCode
{
	NO_ERROR,
	ERROR_DEFAULT,
	STATUS_200 = 200,
	STATUS_201 = 201,
	STATUS_202 = 202,
	STATUS_400 = 400,
	STATUS_403 = 403,
	STATUS_404 = 404,
	STATUS_405 = 405,
	STATUS_413 = 413,
	STATUS_408 = 408,
	STATUS_500 = 500,
	STATUS_505 = 505
} HttpStatusCode;


/**************************************************************/
/*                       FUNCTIONS                            */
/**************************************************************/

std::vector<int> 			parseMultiValueInt(std::istringstream& lineStream);
std::vector<std::string>    parseMultiStringValue(std::istringstream &lineStream);

#endif