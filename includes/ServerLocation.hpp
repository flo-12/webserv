#ifndef SERVERLOCATION_HPP
# define SERVERLOCATION_HPP

# include <string>
# include <vector>
# include <iostream>
# include <sstream> //std::stringstream
# include "common.hpp"

class ServerLocation
{
public:
    // public methods

    // Getters
    std::string             getIndex(void) const;
    std::vector<httpMethod> getMethods(void) const;
    bool                    getAutoindex(void) const;
    std::string             getReturn(void) const;
    std::string             getRoot(void) const;
    std::string             getCgiPath(void) const;
    std::string             getCgiExtension(void) const;

    // Constructors, Assignment, Destructors
    ServerLocation(void);
    ServerLocation(std::stringstream &locationBlock);
    ServerLocation(const ServerLocation& copy);
    ServerLocation& operator=(const ServerLocation& other);
    ~ServerLocation(void);

private:

    // parsing
    void    _parseMethods(std::istringstream &lineStream);
    
    // private data
    std::string                 _index;
    std::vector<httpMethod>     _methods;
    bool                        _autoindex;
    std::string                 _return;
    std::string                 _root;
    std::string                 _cgiPath;
    std::string                 _cgiExtension;
};

std::ostream& operator<<(std::ostream& os, const ServerLocation& serverGeneral);

#endif