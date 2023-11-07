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