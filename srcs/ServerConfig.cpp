#include "ServerConfig.hpp"

/*------------------------------ PUBLIC METHODS -----------------------------*/

/*
*   Builds the complete (internal) path for serving a file by combining the
*   requestUri with the information specified in the location. The requestUri
*   will not contain the part of the locationKey anymore
*   Scenarios:
*   1. requestUri specifies a file 
        => append this file
*   2. requestUri does not specify a file
        => use file specfied in "index" of location
        => if not specified use "index" of general ServerConfig
*   3. Location specifies a "root"
        => use path in "root" as prefix to filename/path
*   4. Location does not specifiy a "root"
        => use "root" member from ther ServerConfig
*/
std::string ServerConfig::getUri(std::string locationKey, std::string requestUri)
{
    // do I have to handle the autoindex here as well?

    bool    hasFilename = true;
    std::string fileName;
    std::string pathPrefix;

    if (requestUri[requestUri.length() -1 ] == '/')
        hasFilename = false;
    
    pathPrefix = _serverLocations[locationKey].getRoot();
    if (pathPrefix.empty())
        pathPrefix = _root;

    if (hasFilename == true)
        fileName = requestUri.substr(locationKey.length() + 1, requestUri.length());
    else 
    {
        fileName = _serverLocations[locationKey].getIndex();
        if (fileName.empty())
            fileName = _index;
    }
    return (pathPrefix + "/" + fileName);
}

/*
*   Gets the URI path from the HTTP request. Searches through the key values of
*   all Locations in the ServerConfig.
*   To do so it first has to extract the requested directory from the URI request.
*   e.g. /home/test/index.html => /home/test. It will subsequently remove more 
*   directories if no matches are found. Since the location "/" will always be
*   provided, this is returned when there are no other matches
*/
std::string ServerConfig::getLocationKey(std::string requestUri)
{
    // do I have to handle the autoindex here as well?

    int         dir_depth = 0;
    std::string directory;

    for (size_t i = 0; i < requestUri.length(); i++) {
        if (requestUri[i] == '/') {
            dir_depth++;
        }
    }
    directory = requestUri;
    while (dir_depth > 0)
    {
        for (std::map<std::string, ServerLocation>::const_iterator it = _serverLocations.begin(); 
            it != _serverLocations.end(); ++it)
        {
            if (it->first == directory)
                return (it->first);
        }
        directory = requestUri.substr(0, directory.rfind('/'));
        dir_depth--;
    }
    return ("/");
}

/*----------------------------- PRIVATE METHODS -----------------------------*/

/*
Takes an IP address as a string in format "127.0.0.1" and translates the four
octets into their integer representation.
*/
unsigned int ServerConfig::_ipStringToInt(const std::string ipAddress) {
    std::vector<unsigned int> parts;
    std::istringstream ipStream(ipAddress);
    std::string part;

    // this is terrible !!!
    if (ipAddress.empty())
        return(0);
    
    while (std::getline(ipStream, part, '.')) 
    {
        unsigned int num = static_cast<unsigned int>(atoi(part.c_str()));
        parts.push_back(num);
    }
    unsigned int ip = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
    (void)ipAddress;
    return (ip);
}

void    ServerConfig::_setDefaultErrorPages(void)
{
    // _errorPage.insert(std::make_pair(404, "error_pages/404.html"));
}

void    ServerConfig::_setDefaultLocations(void)
{
    // _serverLocations.insert(std::make_pair("/", ServerLocation()));
    // _serverLocations.insert(std::make_pair("/cgi", ServerLocation()));
    // _serverLocations["/cgi"].cgiExtension(".py");
    // _serverLocations["/cgi"].cgiPath("/usr/bin/py");
    // _serverLocations["/cgi"].index("script.py");
}

/*
sets default configurations for the server name, error pages and location
*/
void    ServerConfig::_setDefaultConfig(int index)
{
    if (_serverName.empty())
    {
        std::stringstream ss;
        ss << "server_" << index;
        _serverName = ss.str();
    }
    if (_errorPage.empty())
        _setDefaultErrorPages();
    if (_serverLocations.empty())
        _setDefaultLocations();
}

/*
receives a lineStream object with one or multiple key value pairs for the error
pages. Stores them in the std::map<int, std::string> errorPage of ServerConfig
*/

void ServerConfig::_parseErrorPage(std::istringstream &lineStream) {
    int errorCode;
    std::string errorPage;

    while (lineStream)
    {
        if (lineStream >> errorCode >> errorPage) 
            _errorPage[errorCode] = errorPage;
        else
        {
            // handle errors like an incomplete key value pair
        }
    }
}

/*
Receives a stringstream object from the parseConfigFile which contains the 
contents of one server block (server{...}). Proceeds to parse each line. 
Empty lines and lines startung with a "#"" will be skipped. 
*/
void    ServerConfig::_parseServerConfig(std::stringstream &serverBlock)
{
    std::string line;

    while (std::getline(serverBlock, line, ';')) 
    {
        std::istringstream lineStream(line);
        std::string key;

        lineStream >> key;
        if (key.empty())
            continue;
        if (key == "listen")
            _ports = parseMultiValueInt(lineStream);
        else if (key == "server_name")
            lineStream >> _serverName;
        else if (key == "host")
            lineStream >> _host;
        else if (key == "root")
            lineStream >> _root;
        else if (key == "index")
            lineStream >> _index;
        else if (key == "error_page")
            _parseErrorPage(lineStream);
        else
            throw (std::runtime_error("Error, unknown keyword: " + key));
    }
}



/*--------------------------- GETTERS AND SETTERS ---------------------------*/

std::string ServerConfig::getPort(void) const
{
    std::stringstream ss;
    ss << _ports[0];
    return (ss.str());
}

std::vector<int>    ServerConfig::getPorts(void) const
{
    return (_ports);
}

std::string ServerConfig::getHost(void) const
{
    return (_host);
}

std::string ServerConfig::getRoot(void) const
{
    return (_root);
}

std::string ServerConfig::getServerName(void) const
{
    return (_serverName);
}

int ServerConfig::getClientMaxBodySize(void) const
{
    return (_clientMaxBodySize);
}

std::string ServerConfig::getIndex(void) const
{
    return (_index);
}

std::map<int, std::string>  ServerConfig::getErrorPages(void) const
{
    return (_errorPage);
}

std::map<std::string,ServerLocation> ServerConfig::getLocations(void) const
{
    return (_serverLocations);
}


unsigned int    ServerConfig::getDecimalIPaddress(void) const
{
    return (_decimalIPaddress);
}

/*------------------- CONSTRUCTOR, ASSIGNEMENT, DESTRUCTOR ------------------*/

ServerConfig::ServerConfig(void)
{
}


bool compareKeyLength(const std::pair<std::string, ServerLocation>& a, 
    const std::pair<std::string, ServerLocation>& b) 
{
    return a.first.length() < b.first.length();
}

/*
Constructor which is called when a config file is provided.
it parses the content of a previously extracted server{...} block and
saves it as properties of its own class.
*/
ServerConfig::ServerConfig(std::stringstream &serverBlock, int index,
    std::map<std::string, ServerLocation> vecLB)
:   _ports(), _host(""), _root(""),  _serverName(""), 
    _clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE), _index(DEFAULT_INDEX), 
    _errorPage(), _serverLocations(), _decimalIPaddress(0)
{
    _parseServerConfig(serverBlock);
    _decimalIPaddress = _ipStringToInt(_host);
    _serverLocations = vecLB;
    (void)index;

    // if not all arguments are provided set default things ???
    // _setDefaultConfig(index);
}

ServerConfig::ServerConfig(const ServerConfig &copy)
    : _ports(copy._ports), _host(copy._host), _root(copy._root), 
        _serverName(copy._serverName), _clientMaxBodySize(copy._clientMaxBodySize),
        _index(copy._index), _errorPage(copy._errorPage), 
        _serverLocations(copy._serverLocations), _decimalIPaddress(copy._decimalIPaddress)
{
    // std::cout << "Server Config copy constructor called" << std::endl;
}

ServerConfig& ServerConfig::operator=(const ServerConfig &other)
{
    // std::cout << "ServerConfig assignment called" << std::endl;
    if (this != &other)
    {
        _ports = other._ports;
        _host = other._host;
        _root = other._root;
        _serverName = other._serverName;
        _clientMaxBodySize = other._clientMaxBodySize;
        _index = other._index;
        _errorPage = other._errorPage;
        _serverLocations = other._serverLocations;
        _decimalIPaddress = other._decimalIPaddress;
    }
    return (*this);
}

ServerConfig::~ServerConfig(void)
{
}

/*-------------------------- OPERATOR OVERLOADING ---------------------------*/

/*
prints all properties of the ServerConfig class to std::cout
*/
std::ostream& operator<<(std::ostream& os, const ServerConfig& serverConfig) 
{
    
    os << "\033[34m---------- ServerConfig:-----------\033[0m\n";
    os << "Port(s):                 ";
    for (size_t i = 0; i < serverConfig.getPorts().size(); ++i) {
        os << serverConfig.getPorts()[i] << " ";
    }
    os << "\n";
    os << "Server Name:             " << serverConfig.getServerName() << "\n";
    os << "Host:                    " << serverConfig.getHost() << "\n";
    os << "IP (int):                " << serverConfig.getDecimalIPaddress() << "\n";
    os << "Root:                    " << serverConfig.getRoot() << "\n";
    os << "Client Max Body Size:    " << serverConfig.getClientMaxBodySize() << "\n";
    os << "Index:                   " << serverConfig.getIndex() << "\n";
    os << "Error Pages:\n";
    for (std::map<int, std::string>::const_iterator it = serverConfig._errorPage.begin();
            it != serverConfig._errorPage.end(); ++it) 
    {
        os << " " << it->first << ": " << it->second << "\n";
    }
    os << "\n";
    for (std::map<std::string, ServerLocation>::const_iterator it = serverConfig._serverLocations.begin(); 
            it != serverConfig._serverLocations.end(); ++it) {
        os << "\033[92m---------- Location: " << it->first << " -----------\033[0m\n";
        os << it->second;
        os << std::endl;
    }
    os << "\n";
    return os;
}   