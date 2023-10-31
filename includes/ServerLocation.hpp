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

    //Getters and Setters

    // Constructors, Assignment, Destructors
    ServerLocation(std::stringstream &locationBlock);
    ServerLocation(const ServerLocation& copy);
    ServerLocation& operator=(const ServerLocation& other);
    ~ServerLocation(void);

    // Operator Overloading
    friend std::ostream& operator<<(std::ostream& os,
        const ServerLocation& serverGeneral);

private:
    ServerLocation(void);
    
    // private data
    std::string                 _index;
    std::vector<std::string>    _methods;
    bool                        _autoindex;
    std::string                 _return;
    std::string                 _cgiPath;
    std::string                 _cgiExtension;
};

#endif