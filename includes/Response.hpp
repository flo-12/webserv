#include "common.hpp"
#include "CGIHandler.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <string>


class Response
{
	private:
		std::string							_path;
		std::map<std::string, std::string>	_request;
		std::string							_msgHeader;
		std::string							_msgBody;
		ssize_t								_msgLength;

		void		_buildResponse( std::string path );
		void		_buildResponseCGI( std::string path );
		std::string	_readFile( std::string path );

	public:
		Response( std::string path );
		~Response();
	
		std::string	getHeader() const;
		std::string	getBody() const;
		ssize_t		getLength() const;
} ;