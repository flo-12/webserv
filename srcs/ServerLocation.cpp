#include "ServerLocation.hpp"

/*------------------------------ PUBLIC METHODS -----------------------------*/

/*--------------------------- GETTERS AND SETTERS ---------------------------*/


/*------------------- CONSTRUCTOR, ASSIGNEMENT, DESTRUCTOR ------------------*/

ServerLocation::ServerLocation()
{
}

ServerLocation::ServerLocation(std::stringstream &locationBlock)
{
    std::string line;

    while (std::getline(locationBlock, line, ';'))
    {
        std::istringstream  lineStream(line);
        std::string         key;
        std::string         value;

        lineStream >> key;
        if (key.empty())
            continue;
        if (key == "allow_methods")
            _methods = parseMultiStringValue(lineStream);
        else if (key == "autoindex")
        {
            lineStream >> value;
            _autoindex = (value == "on");
        }
        else if (key == "index")
            lineStream >> _index;
        else if (key == "return")
            lineStream >> _return;
        else if (key == "cgi_path")
            lineStream >> _cgiPath;
        else if (key == "cgi_ext")
            lineStream >> _cgiExtension;
        else
            throw (std::runtime_error("Error, unknown keyword: " + key));
    }
}

ServerLocation::ServerLocation(const ServerLocation& copy)
    :   _index(copy._index), _methods(copy._methods), _autoindex(copy._autoindex),
        _return(copy._return), _cgiPath(copy._cgiPath), _cgiExtension(copy._cgiExtension)
{
}

ServerLocation& ServerLocation::operator=(const ServerLocation& other)
{
    if (this != &other)
    {
        _index = other._index;
        _methods = other._methods;
        _autoindex = other._autoindex;
        _return = other._return;
        _cgiPath = other._cgiPath;
        _cgiExtension = other._cgiExtension;
    }
    return (*this);
}

ServerLocation::~ServerLocation(void)
{}

/*-------------------------- OPERATOR OVERLOADING ---------------------------*/

std::ostream& operator<<(std::ostream& os, const ServerLocation& serverLocation) {
    os << "Index:           " << serverLocation._index << "\n";
    os << "Allowed Methods: ";
    for (size_t i = 0; i < serverLocation._methods.size(); ++i) {
        os << serverLocation._methods[i];
        if (i < serverLocation._methods.size() - 1) {
            os << ", ";
        }
    }
    os << "\n";
    os << "Autoindex:       " << (serverLocation._autoindex ? "On" : "Off") << "\n";
    os << "Return:          " << serverLocation._return << "\n";
    os << "CGI path:        " << serverLocation._cgiPath << "\n";
    os << "CGI extension:   " << serverLocation._cgiExtension << "\n";
    return os;
}