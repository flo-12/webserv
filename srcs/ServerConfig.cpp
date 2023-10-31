#include "ServerConfig.hpp"

/*------------------------------ PUBLIC METHODS -----------------------------*/

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

/*------------------- CONSTRUCTOR, ASSIGNEMENT, DESTRUCTOR ------------------*/

ServerConfig::ServerConfig()
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
    for (size_t i = 0; i < serverConfig._ports.size(); ++i) {
        os << serverConfig._ports[i] << " ";
    }
    os << "\n";
    os << "Server Name:             " << serverConfig._serverName << "\n";
    os << "Host:                    " << serverConfig._host << "\n";
    os << "IP (int):                " << serverConfig._decimalIPaddress << "\n";
    os << "Root:                    " << serverConfig._root << "\n";
    os << "Client Max Body Size:    " << serverConfig._clientMaxBodySize << "\n";
    os << "Index:                   " << serverConfig._index << "\n";
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
        std::cout << it->second;
        std::cout << std::endl;
    }
    os << "\n";
    return os;
}   