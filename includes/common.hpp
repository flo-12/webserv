#ifndef COMMON_HPP
# define COMMON_HPP

# include <vector>
# include <string>
# include <sstream>
# include <iostream>
# include <sys/types.h>
# include <sys/stat.h>


/**************************************************************/
/*                        DEFINES                             */
/**************************************************************/

// Colors:
# define RED "\033[0;31m"
# define GREEN "\033[0;32m"
# define YELLOW "\033[0;33m"
# define BABY_BLUE "\033[38;5;111m"
# define BLUE "\033[0;34m"
# define MAGENTA "\033[0;35m"
# define RESET_PRINT "\033[0m"

// Default server configurations:
# define DEFAULT_CLIENT_MAX_BODY_SIZE 3000000
# define DEFAULT_INDEX "index.html"

// Webserv:
# define MAX_CONNECTIONS 500
# define TIMEOUT_POLL 5000
# define TIMEOUT_CGI 3
# define MAX_REQ_SIZE 8192
# define TIMEOUT_RECEIVE 8192

// Response:
# define HTTP_VERSION "HTTP/1.1"
# define HTTP_STATS_CODE_FILE "./database/http_status_codes.csv"

// Debug-Messages:
#define	DEBUG_REQUEST 0
#define DEBUG_REQUEST_INTERNAL 0
#define	DEBUG_REQUEST_HEADER 0
#define	DEBUG_RESPONSE 0
#define	DEBUG_RESPONSE_HEADER 0
#define DEBUG_RESPONSE_ERROR 0
#define	DEBUG_SERVER_STATE 0
#define	DEBUG_SERVER_STATE_DEEP 0
#define	DEBUG_SERVER_STATE_ERROR 0
#define	DEBUG_CONFIG_WARNING 0
#define	DEBUG_SERVER_CONFIG 0
#define	DEBUG_PATHS 0
#define	DEBUG_PRECOND 0
#define DEBUG_CGI 1


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
	STATUS_204 = 204,
	STATUS_206 = 206,
	STATUS_300 = 300,
	STATUS_301 = 301,
	STATUS_302 = 302,
	STATUS_303 = 303,
	STATUS_304 = 304,
	STATUS_307 = 307,
	STATUS_308 = 308,
	STATUS_400 = 400,
	STATUS_403 = 403,
	STATUS_404 = 404,
	STATUS_405 = 405,
	STATUS_408 = 408,
	STATUS_413 = 413,
	STATUS_500 = 500,
	STATUS_504 = 504,
	STATUS_505 = 505
} HttpStatusCode;

typedef enum e_fileExtension
{
	NONE,
	PHP,
	PY,
	OTHER
}	fileExtension;


/**************************************************************/
/*                       FUNCTIONS                            */
/**************************************************************/

std::vector<int>	parseMultiValueInt(std::istringstream& lineStream);
std::string			to_string( ssize_t nbr );
std::string			to_string( HttpStatusCode nbr );
std::string			parseSingleValueString(std::istringstream &lineStream, const std::string &whichLine);
bool				isDirectory(const std::string path);
std::string			extractFileExtension(const std::string &str);
std::string			httpMethodToString(httpMethod method);
void				printDebug( std::string msg, int print, std::string color, int nbr_tabs );

#endif
