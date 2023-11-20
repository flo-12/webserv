# include "common.hpp"

std::string httpMethodToString(httpMethod method) {
    switch (method) {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case DELETE:
            return "DELETE";
        case HEAD:
            return "HEAD";
        case PUT:
            return "PUT";
        case OPTIONS:
            return "OPTIONS";
        case TRACE:
            return "TRACE";
        case CONNECT:
            return "CONNECT";
        default:
            return "NO_TYPE";
    }
}

std::vector<int> parseMultiValueInt(std::istringstream& lineStream) 
{
	    std::vector<int>    values;
	    int                 value;

	    while (lineStream >> value) 
		{
	        values.push_back(value);
	    }
	    return values;
}

std::string	to_string( ssize_t nbr )
{
	std::stringstream ss;
	ss << nbr;

	return ss.str();
}

std::string	to_string( HttpStatusCode nbr )
{
	std::stringstream ss;
	ss << nbr;

	return ss.str();
}

bool isDirectory(const std::string path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        // give an internal server error here
		return false;
    }
    return S_ISDIR(info.st_mode);
}

std::string extractFileExtension(const std::string &str)
{
    size_t dotPosition = str.find_last_of('.');

    if (dotPosition != std::string::npos) 
        return (str.substr(dotPosition));
    else
        return ("");
}

std::string	parseSingleValueString(std::istringstream &lineStream, const std::string &whichLine)
{
    std::string value;
    std::string empty;

    lineStream >> value;
    lineStream >> empty;
    if (!empty.empty())
        throw(std::runtime_error("Error when parsing line '" + whichLine + 
                "' too many values"));
    return (value);
}

/* printDebug:
*	Prints the message passed as argument if the print argument is true (=1)
*	In the color passed as argument.
*/
void	printDebug( std::string msg, int print, std::string color, int nbr_tabs )
{
	if ( print == 1 ) {
		while ( nbr_tabs-- > 0 )
			std::cout << "\t";
		std::cout << color << msg << RESET_PRINT << std::endl;
	}
}
