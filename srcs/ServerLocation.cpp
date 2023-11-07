#include "ServerLocation.hpp"

/*------------------------------ PUBLIC METHODS -----------------------------*/

/*----------------------------- PRIVATE METHODS -----------------------------*/

void    ServerLocation::_parseMethods(std::istringstream &lineStream)
{
    std::string                 value;

    while (lineStream >> value)
    {
        if (value == "GET")
            _methods.push_back(GET);
        else if (value == "POST")
            _methods.push_back(POST);
        else if (value == "DELETE")
            _methods.push_back(DELETE);
        else
            _methods.push_back(NO_TYPE);
    }
}


/*--------------------------- GETTERS AND SETTERS ---------------------------*/

std::string ServerLocation::getIndex(void) const
{
    return (_index);
}

std::vector<httpMethod> ServerLocation::getMethods(void) const
{
    return (_methods);
}

bool    ServerLocation::getAutoindex(void) const
{
    return (_autoindex);
}

std::string ServerLocation::getReturn(void) const
{
    return (_return);
}

std::string ServerLocation::getRoot(void) const
{
    return (_root);
}

std::string ServerLocation::getCgiPath(void) const
{
    return (_cgiPath);
}

std::string ServerLocation::getCgiExtension(void) const
{
    return (_cgiExtension);
}


/*------------------- CONSTRUCTOR, ASSIGNEMENT, DESTRUCTOR ------------------*/

ServerLocation::ServerLocation()
    :   _index(""), _methods(), _autoindex(false), _return(""), _root(""),
        _cgiPath(""), _cgiExtension("")
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
            _parseMethods(lineStream);
        else if (key == "autoindex")
        {
            lineStream >> value;
            _autoindex = (value == "on");
        }
        else if (key == "index")
            lineStream >> _index;
        else if (key == "return")
            lineStream >> _return;
        else if (key == "root")
            lineStream >> _root;
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
        _return(copy._return), _root(copy._root), _cgiPath(copy._cgiPath),
        _cgiExtension(copy._cgiExtension)
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
        _root = other._root;
        _cgiPath = other._cgiPath;
        _cgiExtension = other._cgiExtension;
    }
    return (*this);
}

ServerLocation::~ServerLocation(void)
{}

/*-------------------------- OPERATOR OVERLOADING ---------------------------*/

std::ostream& operator<<(std::ostream& os, const ServerLocation& serverLocation) {
    os << "Index:           " << serverLocation.getIndex() << "\n";
    os << "Allowed Methods: ";
    for (size_t i = 0; i < serverLocation.getMethods().size(); ++i) {
        os << serverLocation.getMethods()[i];
        if (i < serverLocation.getMethods().size() - 1) {
            os << ", ";
        }
    }
    os << "\n";
    os << "Autoindex:       " << (serverLocation.getAutoindex() ? "On" : "Off") << "\n";
    os << "Return:          " << serverLocation.getReturn() << "\n";
    os << "Root:            " << serverLocation.getRoot() << "\n";
    os << "CGI path:        " << serverLocation.getCgiPath() << "\n";
    os << "CGI extension:   " << serverLocation.getCgiExtension() << "\n";
    return os;
}