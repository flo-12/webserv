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


/**************************************************************/
/*                        STRUCTS                             */
/**************************************************************/

typedef struct s_ipPort
{
    unsigned int    ip;
    int             port;
}   t_ipPort;


/**************************************************************/
/*                       FUNCTIONS                            */
/**************************************************************/

std::vector<int> 			parseMultiValueInt(std::istringstream& lineStream);
std::vector<std::string>    parseMultiStringValue(std::istringstream &lineStream);

#endif