#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include <string>
# include <vector>
# include <iostream>
# include <sstream>
# include <map>
# include <cstdlib>
# include <algorithm> // for std::sort
# include "ServerLocation.hpp"
# include "common.hpp"

class ServerConfig {
    friend class ConfigParser;
public:

    // public methods - for building Response 
    std::string                             getLocationKey(std::string requestUri);
    std::string                             getUri(std::string locationKey, std::string requestURI);

    // Getters
    std::string                             getPort(void) const;
    std::vector<int>                        getPorts(void) const;
    std::string                             getHost(void) const;
    std::string                             getRoot(void) const;
    std::string                             getServerName(void) const;
    int                                     getClientMaxBodySize(void) const;
    std::string                             getIndex(void) const;
    std::map<int, std::string>              getErrorPages() const;
    std::map<std::string,ServerLocation>    getLocations(void) const;
    unsigned int                            getDecimalIPaddress(void) const;

    // Constructors, Assignment, Destructors
    ServerConfig(std::stringstream &serverBlock, int index, 
        std::map<std::string, ServerLocation> vecLB);
    ServerConfig(const ServerConfig &copy);
    ServerConfig& operator=(const ServerConfig &other);
	ServerConfig(void);
    ~ServerConfig(void);

    // Operator Overloading
    friend std::ostream& operator<<(std::ostream& os,
        const ServerConfig& serverConfig);

private:
    // parsing
    void            _setDefaultErrorPages(void);
    void            _setDefaultLocations(void);
    void            _setDefaultConfig(int index);
    void            _parseErrorPage(std::istringstream &lineStream);
    void            _parseServerConfig(std::stringstream &serverBlock);
    unsigned int    _ipStringToInt(const std::string ipAddress);
    
    

    //private data
    std::vector<int>                        _ports;
    std::string                             _host;
    std::string                             _root;
    std::string                             _serverName;
    int                                     _clientMaxBodySize;
    std::string                             _index;
    std::map<int, std::string>              _errorPage; // rename to errorPages
    std::map<std::string, ServerLocation>   _serverLocations;
    unsigned int                            _decimalIPaddress;
    
    // bool    validateConfig();
};

# endif