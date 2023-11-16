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

/*
*   Checks prerequsites for a path, in case of error returns false; Prerequisites:
        - Methods must be either GET, POST, DELETE, also cannot be empty
        - If autoindex and index are mutally exclusive
        - Status codes outside of the range of 301 to 308 or return
*/
std::string ServerLocation::validate(void) const
{
    std::string errorReturn("");

    if (_methods.empty())
        errorReturn += "\t must specify 'allow_methods'\n";

    // unknown method
    size_t i;
    for (i = 0; i < _methods.size(); i++)
    {
        if (_methods[i] == GET || _methods[i] == POST || _methods[i] == DELETE)
            continue;
        else
            break;
    }
    if (i != _methods.size())
        errorReturn += "\t method not supported. Please use GET, POST, DELETE\n";

    // autoindex <-> index
    if (_autoindex == true && !_index.empty())
        errorReturn += "\t autoindex active, cannot serve a default index\n";

    // return HTTP status code
    if (_return.first != 0 && (_return.first < 301 || _return.first > 308))
        errorReturn += "\t HTTP status code for redirection is out or rage (301 - 308)\n";


    return (errorReturn);
}

/*
*   Checks whether a location that will be used for CGI:
*       - allows at least one methos
*       - does not specify an autoindex, index, return 
*       - specifies a cgiPath (the executable like php or python) and a 
            root folder where the Cgi should be executed  
*/
std::string ServerLocation::validateCgiLocation(void) const
{
    std::string errorReturn("");

    if (_methods.empty())
        errorReturn += "\t must specify 'allow_methods'\n";

    // unknown method
    size_t i;
    for (i = 0; i < _methods.size(); i++)
    {
        if (_methods[i] == GET || _methods[i] == POST || _methods[i] == DELETE)
            continue;
        else
            break;
    }
    if (i != _methods.size())
        errorReturn += "\t method not supported. Please use GET, POST, DELETE\n";
    
    // no index, autoindex or return
    if (!_index.empty() || _autoindex == true || _return.first != 0)
        errorReturn += "cannot specify index, autoindex or return in CGI\n";
    
    // must specify a root and a path 
    if (_root.empty() || _cgiPath.empty())
        errorReturn += "CGI must specify a root and a cgi_path\n";

    return (errorReturn);
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

std::pair<HttpStatusCode, std::string> ServerLocation::getReturn(void) const
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

bool    ServerLocation::getIsCgi(void) const
{
    if (_cgiPath.empty())
        return (false);
    else
        return (true);
}

void    ServerLocation::setMethods(std::vector<httpMethod> methods)
{
    _methods = methods;
}

/*------------------- CONSTRUCTOR, ASSIGNEMENT, DESTRUCTOR ------------------*/

ServerLocation::ServerLocation()
    :   _index(""), _methods(), _autoindex(false), _root(""),
        _cgiPath("") 
{
}

std::pair<HttpStatusCode, std::string> parseStatusCodeStringPair(std::istringstream &lineStream,
                                        const std::string &key)
{
    int                                     statusCode;
    std::string                             str;
    std::pair<HttpStatusCode, std::string>  newPair;

    (void) key;

    if (lineStream >> statusCode >> str)
    {
        return (std::make_pair(static_cast<HttpStatusCode>(statusCode), str));
    }
    else
    {
            throw(std::runtime_error("Error, incomplete key value pair"));
    }
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
            _index = parseSingleValueString(lineStream, key);
        else if (key == "return")
            _return = parseStatusCodeStringPair(lineStream, key);
        else if (key == "root")
            _root = parseSingleValueString(lineStream, key);
        else if (key == "cgi_path")
            _cgiPath = parseSingleValueString(lineStream, key);
        else if (key == "is_cgi")
        {
            lineStream >> value;
            _autoindex = (value == "false");
        }
        else
            throw (std::runtime_error("Error, unknown keyword: " + key));
    }
}

ServerLocation::ServerLocation(const ServerLocation& copy)
    :   _index(copy._index), _methods(copy._methods), _autoindex(copy._autoindex),
        _return(copy._return), _root(copy._root), _cgiPath(copy._cgiPath) //, isCgi(copy._isCgi)
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
        //_isCgi = other._isCgi;
    }
    return (*this);
}

ServerLocation::~ServerLocation(void)
{}

/*-------------------------- OPERATOR OVERLOADING ---------------------------*/

std::ostream& operator<<(std::ostream& os, const ServerLocation& serverLocation) {
    os << "Allowed Methods: ";
    for (size_t i = 0; i < serverLocation.getMethods().size(); ++i) {
        os << httpMethodToString(serverLocation.getMethods()[i]);
        if (i < serverLocation.getMethods().size() - 1) {
            os << ", ";
        }
    }
    os << "\n";
    os << "Autoindex:       " << (serverLocation.getAutoindex() ? "On" : "Off") << "\n";
    std::pair<HttpStatusCode, std::string> returnCopy = serverLocation.getReturn();
    if (!serverLocation.getIndex().empty())
        os << "Index:           " << serverLocation.getIndex() << "\n";
    if (returnCopy.first != 0)
        os << "Return:          " << returnCopy.first << " " << returnCopy.second << "\n";
    if (!serverLocation.getRoot().empty())
        os << "Root:            " << serverLocation.getRoot() << "\n";
    
    // figure out this one later, it's tooooo frustrating
    // if (serverLocation.getIsCgi() == true)
    // {
    // os << "Is CGI:          " << (serverLocation.getIsCgi() ? "TRUE" : "FALSE") << "\n";
    // }
    if (!serverLocation.getCgiPath().empty())
        os << "CGI path:        " << serverLocation.getCgiPath() << "\n";
    return os;
}