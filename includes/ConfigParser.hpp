#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <iostream>
# include <sstream>
# include <fstream>
# include "common.hpp"
# include "ServerConfig.hpp"

class ConfigParser
{
public:
    // public methods
    std::vector<t_ipPort>  getIpPort(void);
    void                    print(void) const;

    // Getters and Setters
    std::vector<ServerConfig> serverConfigs(void) const;

    // Constructors, Assignment, Destructors
    ConfigParser(std::string fileName);
    ConfigParser(const ConfigParser& copy);
    ConfigParser& operator=(const ConfigParser& other);
    ~ConfigParser(void);

private:
    ConfigParser(void);
	
    // private methods
    void                _parseConfigFile(std::ifstream& configFile);
    std::string         _extractLocationKey(std::string &line);
    std::string         _extractLocationBlock(std::string &line, std::ifstream &configFile);

    // data
    std::vector<ServerConfig>   _serverConfigs;
};

#endif