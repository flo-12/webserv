# include "common.hpp"

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