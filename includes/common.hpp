#ifndef COMMON_HPP
# define COMMON_HPP

# include <vector>
# include <string>
# include <sstream>

// Default server configurations:
# define DEFAULT_HOST "127.0.0.1"
# define DEFAULT_ROOT "/www"
# define DEFAULT_CLIENT_MAX_BODY_SIZE 300000
# define DEFAULT_INDEX "index.html"
# define DEFAULT_PORT 8000

typedef struct s_ip_port
{
    unsigned int    ip;
    int             port;
}   t_ip_port;

std::vector<int> parseMultiValueInt(std::istringstream& lineStream);
std::vector<std::string>    parseMultiStringValue(std::istringstream &lineStream);

#endif