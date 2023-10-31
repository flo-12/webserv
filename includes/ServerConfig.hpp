#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include <string>
# include <vector>
# include <iostream>
# include <sstream>
# include <map>
# include <cstdlib>
# include "ServerLocation.hpp"
# include "common.hpp"

class ServerConfig {
    friend class ConfigParser;
public:

    // public methods

    // Constructors, Assignment, Destructors
    ServerConfig(std::stringstream &serverBlock, int index, 
        std::map<std::string, ServerLocation> vecLB);
    ServerConfig(const ServerConfig &copy);
    ServerConfig& operator=(const ServerConfig &other);
    ~ServerConfig(void);

    // Operator Overloading
    friend std::ostream& operator<<(std::ostream& os,
        const ServerConfig& serverConfig);

private:
    //private methods
    void            _setDefaultErrorPages(void);
    void            _setDefaultLocations(void);
    void            _setDefaultConfig(int index);
    void            _parseErrorPage(std::istringstream &lineStream);
    void            _parseServerConfig(std::stringstream &serverBlock);
    unsigned int    _ipStringToInt(const std::string ipAddress);
    
    ServerConfig(void); // should not be instanciated

    //private data
    std::vector<int>                        _ports;
    std::string                             _host;
    std::string                             _root;
    std::string                             _serverName;
    int                                     _clientMaxBodySize;
    std::string                             _index;
    std::map<int, std::string>              _errorPage;
    std::map<std::string, ServerLocation>   _serverLocations;
    unsigned int                            _decimalIPaddress;
    
    // bool    validateConfig();
};

# endif