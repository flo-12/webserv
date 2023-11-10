#include "ServerConfig.hpp"

/*------------------------------ PUBLIC METHODS -----------------------------*/

/*
*   Receives a location key, that will always match a Location specfied in the
*   server config. Checks whether ther is a "return" field specfied, to define
*   the redirection. If there is no return (i.e. redirection) for the current
*   location, returns a StatusCode of 0 and an empty String
*/
std::pair<HttpStatusCode, std::string>   ServerConfig::getRedirection(std::string locationKey)
{
    std::pair<std::string, ServerLocation> location;

    return(_serverLocations[locationKey].getReturn());

}

/*
*   Given the HTTP status code, connects the root directory of the server with
*   the directory of the error page. If no error page is found, an empty string
*   is returned
*/
std::string ServerConfig::retrievePathErrorPage(const HttpStatusCode errorStatus) 
{
    std::string errorDir;

    errorDir = _errorPage[errorStatus];
    if (errorDir.empty())
        return ("");
    else
    {
        return (_root + "/" + errorDir);
    }
}

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
*   When the location specifies an autoindex, a '/' is appended at the end
*   instead of a file
*/
std::string ServerConfig::getUri(std::string locationKey, std::string requestUri)
{
    // bool    hasFilename = true;
    std::string fileName;
    std::string root;
    std::string pathSuffix;

    // get root for path
    root = _serverLocations[locationKey].getRoot();
    if (root.empty())
        root = _root;

	// remove locationKey from string
    pathSuffix = requestUri.substr(locationKey.length(), requestUri.length());

    // check if location has autoindex
    if (requestUri[requestUri.length() - 1 ] == '/' &&
            _serverLocations[locationKey].getAutoindex() == true)
    {
		if (pathSuffix[pathSuffix.size() - 1 == '/'])
            return (root + pathSuffix);
        else
            return (root + pathSuffix + "/");
    }
    // check if Uri already specified a file, if not add based on "index"
    if (pathSuffix.empty() || pathSuffix == "/")
    {
        fileName = _serverLocations[locationKey].getIndex();
        if (fileName.empty())
            fileName =  "/" + _index;
    }
    // std::cout << "Prefix " << root << std::endl; 
    // std::cout << "Suffix " << pathSuffix << std::endl; 
    // std::cout << "FileName " << fileName << std::endl; 

    if (pathSuffix[0] != '/')
        pathSuffix = "/" + pathSuffix;
    return (root + pathSuffix + fileName);
}

/*
*   Gets the URI path from the HTTP request. Searches through the key values of
*   all Locations in the ServerConfig.
*   To do so it first has to extract the requested directory from the URI request.
*   e.g. /home/test/index.html => /home/test. It will subsequently remove more 
*   directories if no matches are found. Since the location "/" will always be
*   provided, this is returned when there are no other matches.
*/
std::string ServerConfig::getLocationKey(std::string requestUri)
{
    std::string directory;
    std::string fileExt;

    // test if request URI matches CGI
    fileExt = extractFileExtension(requestUri);
    if (!fileExt.empty())
    {
        for (std::map<std::string, ServerLocation>::const_iterator it = _serverLocations.begin(); 
            it != _serverLocations.end(); ++it)
        {
            if (it->first == fileExt)
                return (it->first);
        }
    }

    // test if URI matches path in Locations
    directory = requestUri;
    while (!directory.empty())
    {
        size_t lastSlash = directory.find_last_of('/');
        directory = directory.substr(0, lastSlash);

        for (std::map<std::string, ServerLocation>::const_iterator it = _serverLocations.begin(); 
            it != _serverLocations.end(); ++it)
        {
            if (it->first == directory)
                return (it->first);
        }
    }
    return ("/");
}

/*----------------------------- PRIVATE METHODS -----------------------------*/

/*
Takes an IP address as a string in format "127.0.0.1" and translates the four
octets into their integer representation.
*/
unsigned int ServerConfig::_ipStringToInt(const std::string ipAddress) {
    std::vector<unsigned int>   parts;
    std::istringstream          ipStream(ipAddress);
    std::string                 part;
    unsigned int                ip;
    int                         count = 0;

    // this is terrible !!!
    if (ipAddress.empty())
        return(0);
    while (std::getline(ipStream, part, '.')) 
    {
        count++;
        unsigned int num = static_cast<unsigned int>(atoi(part.c_str()));
        parts.push_back(num);
    }
    if (count != 4)
        throw(std::runtime_error("Error, IP address has invalid format"));
    ip = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
    return (ip);
}

/*
*   compares if there has been a custom route for the error pages. If not
*   adds a default route for. Returns true if any routes have been set.
*/
bool    ServerConfig::_setDefaultErrorPages(void)
{
    int numDefaultErrorLocations = 7;
    bool    b = false;

    HttpStatusCode arrErrorCodes[] = {
        static_cast<HttpStatusCode>(400),
        static_cast<HttpStatusCode>(403),
        static_cast<HttpStatusCode>(404),
        static_cast<HttpStatusCode>(405),
        static_cast<HttpStatusCode>(408),
        static_cast<HttpStatusCode>(500),
        static_cast<HttpStatusCode>(504)
    };
    std::string arrErrorDefaultLocations[] = {
        "/error_pages/400.html",
        "/error_pages/403.html",
        "/error_pages/404.html",
        "/error_pages/405.html",
        "/error_pages/408.html",
        "/error_pages/500.html",
        "/error_pages/504.html",
    };

    for (int i = 0; i < numDefaultErrorLocations; i ++)
    {
        if (_errorPage.find(arrErrorCodes[i]) == _errorPage.end())
        {
            std::cout   << YELLOW << "WARNING: " << RESET_PRINT << "no error page " 
                        << arrErrorCodes[i] << " provided, default set to " 
                        << arrErrorDefaultLocations[i] << std::endl;
            _errorPage.insert(std::make_pair(arrErrorCodes[i], 
                                                arrErrorDefaultLocations[i]));
            b = true;
        }
    }
    return (b);
}

bool    ServerConfig::_setDefaultLocations(void)
{
    bool                                                    b = false;
    std::map<std::string, ServerLocation>::const_iterator   it;
    
    it = _serverLocations.find("/");
    if (it == _serverLocations.end())
    {
        std::cout   << YELLOW << "Warning: " << RESET_PRINT << "no location "
                    << "/" << " provided, setting default\n";
       _serverLocations.insert(std::make_pair("/", ServerLocation()));
       b = true;
    }
    it = _serverLocations.find("/uploads");
    if (it == _serverLocations.end())
    {
        std::cout   << YELLOW << "Warning: " << RESET_PRINT << "no location "
                    << "/uploads" << " provided, setting default\n";
       _serverLocations.insert(std::make_pair("/uploads", ServerLocation())); 
        b = true;
    }
    return (b);
}

/*
sets default configurations for the server name, error pages and location
*/
void    ServerConfig::_setDefaultConfig(int index)
{
    (void)index;

    _setDefaultErrorPages(); 
    _setDefaultLocations();
        // std::cout   << YELLOW << " SERVER CONFIGURATION [" << index + 1
        //             << "] " << RESET_PRINT << " \n";
    
    // display warings here as well
    if (_serverName.empty())
        _serverName = "server_" +  to_string(index);
    if (_clientMaxBodySize == 0)
        _clientMaxBodySize = 300000;
    if (_index.empty())
        _index = "index.html";
}

/*
receives a lineStream object with one or multiple key value pairs for the error
pages. Stores them in the std::map<int, std::string> errorPage of ServerConfig
*/
void ServerConfig::_parseErrorPages(std::istringstream &lineStream) {
    int errorCode;
    std::string errorPage;

    while (lineStream)
    {
        if (lineStream >> errorCode >> errorPage)
        {
            _errorPage[ static_cast<HttpStatusCode>(errorCode)] = errorPage;
        }
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
            _serverName = parseSingleValueString(lineStream, key);
        else if (key == "host")
            _host = parseSingleValueString(lineStream, key);
        else if (key == "root")
            _root = parseSingleValueString(lineStream, key);
        else if (key == "index")
            _index = parseSingleValueString(lineStream, key)    ;
        else if (key == "error_page")
            _parseErrorPages(lineStream);
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

std::map<HttpStatusCode, std::string>  ServerConfig::getErrorPages(void) const
{
    // adds the "root" prefix to each errorPages path
    std::map<HttpStatusCode, std::string>   copyErrorPages(_errorPage);

    for (std::map<HttpStatusCode, std::string>::iterator it = copyErrorPages.begin();
            it != copyErrorPages.end(); ++it)
    {
        it->second = _root + it->second;
    } 
    return (copyErrorPages);
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
    std::string error("");

    _parseServerConfig(serverBlock);
    _decimalIPaddress = _ipStringToInt(_host);
    _serverLocations = vecLB;

    _setDefaultConfig(index);
    
    // validation of ServerLocation
    for (std::map<std::string, ServerLocation>:: const_iterator it = _serverLocations.begin();
            it != _serverLocations.end(); it++)
    {
        error += it->second.validate();
    }

    // I probably need a seperate validation for the CGI location

    if (!error.empty())
        throw(std::runtime_error( "\033[0;31mError\033[0m while parsing config file \n" + error));
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
    std::map<HttpStatusCode, std::string>   errorPageCopy(serverConfig.getErrorPages());
    for (std::map<HttpStatusCode, std::string>::const_iterator it = errorPageCopy.begin();
            it != errorPageCopy.end(); ++it) 
    {
        os << " " << it->first << ": " << it->second << "\n";
    }
    os << "\n";
    std::map<std::string, ServerLocation> serverLocationCopy(serverConfig.getLocations());
    for (std::map<std::string, ServerLocation>::const_iterator it = serverLocationCopy.begin(); 
            it != serverLocationCopy.end(); ++it) {
        os << "\033[92m---------- Location: " << it->first << " -----------\033[0m\n";
        os << it->second;
        os << std::endl;
    }
    os << "\n";
    return os;
}   