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

std::vector<std::string>    parseMultiStringValue(std::istringstream &lineStream)
{
    std::vector<std::string>    values;
    std::string                 value;

    while (lineStream >> value)
    {
        values.push_back(value);
    }
    return (values);
}
