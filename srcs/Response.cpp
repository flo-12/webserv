#include "Response.hpp"


/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

Response::Response( std::string _path )
	: _path(_path)
{
	_buildResponse( _path );
}

/* Response::Response( std::map<std::string, std::string> )
	: 
{
	_buildResponse( _path );
} */

Response::~Response()
{
	
}


/**************************************************************/
/*                     GETTER METHODS                         */
/**************************************************************/

std::string	Response::getHeader() const
{
	return _msgStatusLine + _msgHeader;
}

std::string	Response::getBody() const
{
	return _msgBody;
}

ssize_t		Response::getLength() const
{
	return _msgLength;
}



/**************************************************************/
/*                    PRIVATE METHODS                         */
/**************************************************************/

/* buildResponse():
*	Build the response corresponding to the _request and stores it in _response.
*	1st draft contains:
*		- filling out all values from _request
*		- "Hello World" in the body
*		- status line ("HTTP/1.1 200 OK"), "Content-Length" (text/html) and "Content-Length" in the header
*/
void	Response::_buildResponse( std::string path )
{
	std::string prefix1 = "/";
	std::string prefix2 = "/surfer.jpeg";
	std::string prefix3 = "/giga-chad-theme.mp3";

	//if (_request.buffer.find(prefix1) == 0)
	if ( path == prefix1 )
	{
		_msgBody = _readFile( "./html" + path + "index.html" );
		//_msgBody = "Hello World\n";
		std::stringstream	strStream;
		strStream << (_msgBody.length());

		_msgStatusLine = PROTOCOL_VERSION; 
		_msgStatusLine += " 200 OK\r\n";
		_msgHeader += "Content-Type: text/html\r\n";	
		_msgHeader += "Content-Length: " + strStream.str() + "\r\n";
		_msgHeader += "Connection: close\r\n";
		_msgHeader += "\r\n";
	}
	else if ( path == prefix2 )
	{
		_msgBody = _readFile( "./html" + path );
		std::stringstream	strStream;
		strStream << (_msgBody.length());

		_msgStatusLine = PROTOCOL_VERSION; 
		_msgStatusLine += " 200 OK\r\n";
		_msgHeader += "Content-Type: image/*\r\n";
		_msgHeader += "Content-Length: " + strStream.str() + "\r\n";
		_msgHeader += "Connection: close\r\n";
		_msgHeader += "\r\n";
	}
	else if ( path == prefix3 )
	{
		_msgBody = _readFile( "./html" + path );
		std::stringstream	strStream;
		strStream << (_msgBody.length());

		_msgStatusLine = PROTOCOL_VERSION; 
		_msgStatusLine += " 200 OK\r\n";
		_msgHeader += "Content-Type: audio/*\r\n";
		_msgHeader += "Content-Length: " + strStream.str() + "\r\n";
		_msgHeader += "Connection: close\r\n";
		_msgHeader += "\r\n";
	}
	_msgLength = _msgStatusLine.length() + _msgHeader.length() + _msgBody.length();
}

/* FOR CGI-TESTING */
void	Response::_buildResponseCGI( std::string path )
{
	path = "";
	std::string output;
	CGIHandler	CGIHandler;
	_msgBody = CGIHandler.execute();
	std::stringstream	strStream;
	strStream << (_msgBody.length());

	_msgStatusLine = "HTTP/1.1 200 OK\r\n";
	_msgHeader += "Content-Type: text/html\r\n";
	_msgHeader += "Content-Length: " + strStream.str() + "\r\n";
	_msgHeader += "\r\n";
	_msgLength = _msgStatusLine.length() + _msgHeader.length() + _msgBody.length();
}


std::string	Response::_readFile( std::string path )
{
	//std::cout << "Reading file: " << path << std::endl;
	std::string		line;
	std::string		body;
	std::ifstream	ifs(path.c_str());

	if ( !ifs.is_open() )
		throw std::runtime_error("Error: readFile() failed");

	while ( getline( ifs, line ) )
		body += line + "\n";
	ifs.close();

	return body;
}

