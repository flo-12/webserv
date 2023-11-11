#ifndef SERVERLOCATION_HPP
# define SERVERLOCATION_HPP

# include <string>
# include <vector>
# include <utility>
# include <iostream>
# include <sstream> //std::stringstream
# include "common.hpp"

class ServerLocation
{
public:
    // validation
    std::string validate(void) const;

    // Getters
    std::string                             getIndex(void) const;
    std::vector<httpMethod>                 getMethods(void) const;
    bool                                    getAutoindex(void) const;
    std::pair<HttpStatusCode, std::string>  getReturn(void) const;
    std::string                             getRoot(void) const;
    std::string                             getCgiPath(void) const;
    bool                                    getIsCgi(void) const;

    // Constructors, Assignment, Destructors
    ServerLocation(void);
    ServerLocation(std::stringstream &locationBlock);
    ServerLocation(const ServerLocation& copy);
    ServerLocation& operator=(const ServerLocation& other);
    ~ServerLocation(void);

private:
    std::string                             _index;
    std::vector<httpMethod>                 _methods;
    bool                                    _autoindex;
    std::pair<HttpStatusCode, std::string>  _return;
    std::string                             _root;
    std::string                             _cgiPath;
    // bool                                    _isCgi;

    // parsing
    void    _parseMethods(std::istringstream &lineStream);
};

std::ostream& operator<<(std::ostream& os, const ServerLocation& serverGeneral);

#endif