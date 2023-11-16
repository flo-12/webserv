#include "Response.hpp"


/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

/* Response():
*	Default constructor to set the message status line
*	to the given http status code.
*/
Response::Response( HttpStatusCode httpStatus, ServerConfig config )
	: _config(config), _httpVersionAllowed(HTTP_VERSION), _msgBody(""), _msgBodyLength(0)
{
	_readHttpStatusCodeDatabase();
	if ( _httpStatusCodeLookup.empty() )
		throw std::runtime_error("Error: http status code database empty");
	
	_setMsgStatusLine( httpStatus );
	
	_readErrorPage( httpStatus );
}

/* Response():
*	Constructor to build the response message.
*		1) Check preconditions
*		2) Check for redirection
*		3) Check for method and call corresponding handler	
*/
Response::Response( RequestParser request, ServerConfig config )
	: _request(request), _config(config), _httpVersionAllowed(HTTP_VERSION), _msgBody(""), _msgBodyLength(0)
{
	_readHttpStatusCodeDatabase();
	if ( _httpStatusCodeLookup.empty() )
		throw std::runtime_error("Error: http status code database empty");

	std::cout << RED << request.getFormObject().body << RESET_PRINT << std::endl;

	_method = request.getMethod();
	_setPaths( request.getPath() );
	std::cout << RED << _paths.responseUri << RESET_PRINT << std::endl;

	if ( _method != POST && !_checkPreconditions( ) ) // check again the _method!=POST
	{
		_readErrorPage( _msgStatusLine.statusCode );
	}
	else if ( _checkRedirection( ) )
		;
	else if ( _method == GET )
		_handleGet();
	else if ( _method == POST )
		_handlePost();
	else if ( _method == DELETE )
		_handleDelete();
}

Response::~Response()
{
	
}


/**************************************************************/
/*                    PRIVATE METHODS                         */
/**************************************************************/

/* _setPaths:
*	Sets the paths for the response depending on the request URI and the config file.
*/
void	Response::_setPaths( std::string reqUri )
{
	_paths.requestUri = reqUri;
	_paths.confLocKey = _config.getLocationKey( reqUri );
	_paths.responseUri = _config.getUri( _paths.confLocKey, reqUri );
	std::cout << "Response path: \"" << _paths.responseUri << "\"" << std::endl;
}

/* _setMsgStatusLine:
*	Sets the message status line to the http-version and the given http 
*	status code (code + phrase).
*/
void	Response::_setMsgStatusLine( HttpStatusCode httpStatus )
{
	_msgStatusLine.protocolVersion = HTTP_VERSION;

	_msgStatusLine.statusCode = httpStatus;
	std::map<int, std::string>::iterator it = _httpStatusCodeLookup.find(_msgStatusLine.statusCode);
	if ( it != _httpStatusCodeLookup.end() )
		_msgStatusLine.reasonPhrase = it->second;
	else
		_msgStatusLine.reasonPhrase = "";
}

/* _checkPreconditions:
*	Checks if the request meets all preconditions:
*		- path/file exists ⇒ 404 Not Found
*		- right access to path/file (GET->read | ???POST->write??? | DELETE->execute) ⇒ 403 Forbidden
*		- method of request invalid ⇒ 405 Method Not Allowed
*		- HTTP version supported (== 1.1) ⇒ 505 HTTP Version Not Supported
*		- request body size valid (<= confifg.clientMaxBodySize) ⇒ 413 Content Too Large
*		- No or wrong “Host” field in request ⇒ 400 Bad Request
*		- Content length smaller 0 ⇒ 400 Bad Request
*	Returns true if all preconditions are met, false otherwise.
*/
bool	Response::_checkPreconditions()
{
	// check if path/file exists
	// differentiate for DELETE and GET <--- TODO
	if ( access(_paths.responseUri.c_str(), F_OK) != 0 ) {
		std::cerr << YELLOW << "File not found: " << _paths.responseUri << RESET_PRINT << std::endl;
		_readErrorPage( STATUS_404 );
		return false;
	}

	// check if right access to path/file
	if ( (_method == GET && access(_paths.responseUri.c_str(), R_OK) != 0 ) 
		|| (_method == DELETE && access(_paths.responseUri.c_str(), X_OK) != 0 ) ) {
			std::cerr << YELLOW << "No access to file: " << _paths.responseUri << RESET_PRINT << std::endl;
		_readErrorPage( STATUS_403 );
		return false;
	}

	// check if method is allowed
	std::vector<httpMethod> methods = _config.getLocations()[_paths.confLocKey].getMethods();

	for ( std::vector<httpMethod>::iterator it = methods.begin(); it != methods.end(); it++ ) {
		if ( *it == _method )
			break ;
		else if ( it + 1 == methods.end() ) {
			std::cerr << YELLOW << "Method not allowed: " << _method << RESET_PRINT << std::endl;
			_readErrorPage( STATUS_405 );
			return false;
		}
	}

	// check if HTTP version is supported
	if ( _request.getProtocol() != _httpVersionAllowed ) {
		std::cerr << YELLOW << "HTTP version not supported: \"" << _request.getProtocol() << "\"" << RESET_PRINT << std::endl;
		_readErrorPage( STATUS_405 );
		return false;
	}

	// check if request body size is valid
	if ( _request.getContentLength() > _config.getClientMaxBodySize() ) {	// ask Linus to return ssize_t!!
		std::cerr << YELLOW << "Request body too large: " << _request.getContentLength() << RESET_PRINT << std::endl;
		_readErrorPage( STATUS_413 );
		return false;
	}

	// check if Host field is present
	if ( _request.getHost() != _config.getHost() + ":" + _config.getPort()) {
		std::cerr << YELLOW << "Host field not present or wrong: " << _request.getHost() << RESET_PRINT << std::endl;
		_readErrorPage( STATUS_400 );
		return false;
	}

	// check if content length is smaller 0
	if ( _request.getContentLength() < static_cast<ssize_t>(0) ) {
		std::cerr << YELLOW << "Content length smaller 0: " << _request.getContentLength() << RESET_PRINT << std::endl;
		_readErrorPage( STATUS_400 );
		return false;
	}

	return true;
}

/* _checkRedirection:
*	Checks if the request URI is a redirection.
*		- yes ⇒ response with Location and 30X status code
*	Returns true if the request URI is a redirection, false otherwise.
*/
bool	Response::_checkRedirection()
{
	std::pair<HttpStatusCode, std::string>	redirection = _config.getRedirection( _paths.confLocKey );

	if ( redirection.first >= STATUS_300 && redirection.first <= STATUS_308) {
		_setMsgStatusLine( redirection.first );
		_msgHeader["Location"] = redirection.second;
		return true;
	}
	return false;
}

/* _handleGet:
*	Handles a GET request.
*		1) Check if CGI is needed
*			- yes ⇒ call CGI
*		2) [no] Create GET response
*			- read file from path and store in _msgBody and set _msgBodyLength
*			- fill out _msgStatusLine and _msgHeader
*/
void	Response::_handleGet()
{
	// Check autoindex
	if (_paths.responseUri[_paths.responseUri.length() - 1] == '/')
	{
		_readDir( _paths.responseUri );
		if ( !_readDir( _paths.responseUri ) )
			_readErrorPage( STATUS_500 );
		else
			_setMsgStatusLine( STATUS_200 );
	}
	else if ( _isCgiNeeded() ) {
		// call CGI
		try {
			CGIHandler	CGIHandler( _request );
			_msgBody = CGIHandler.getBody();	// change: execute is called by default constructor and CGIHandler has a getBody method
			_msgBodyLength = CGIHandler.getBodyLength();	// change: CGIHandler.getBodyLength()
			_setMsgStatusLine( STATUS_200 );	// check for errors!!!!
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			//_setMsgStatusLine( STATUS_400 );
		}
	}
	else
	{
		// create GET response
		if ( !_readFile( _paths.responseUri ) )
			_readErrorPage( STATUS_500 );
		else
			_setMsgStatusLine( STATUS_200 );
	}

	//_msgHeader["Content-Type"] = ;
	_msgHeader["Content-Length"] = to_string(_msgBodyLength);
}

/* _handlePost:
*	Handles a POST request.
*		- if CGI is needed ⇒ call CGI
*		- else: Creates POST response for
*			- upload file (create file)
*			- submit form without file
*/
void	Response::_handlePost()
{
	/* if ( cgiNeede() )
		// call CGI */
	
	// Prototype for file upload
	/* std::cout << " ++++++++++++++++++++++ POST request ++++++++++++++++++++++" << std::endl;
	std::cout << _request.getBody() << std::endl;
	std::cout << " +++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl; */

	if ( _isCgiNeeded() ) {
		// call CGI
		CGIHandler	CGIHandler( _request );
		_msgBody = CGIHandler.getBody();	// change: execute is called by default constructor and CGIHandler has a getBody method
		_msgBodyLength = CGIHandler.getBodyLength();	// change: CGIHandler.getBodyLength()
		_setMsgStatusLine( STATUS_200 );	// check for errors!!!!
		_msgHeader["Content-Length"] = to_string(_msgBodyLength);
	}
	else {
		std::string fileName(_request.getFormObject().fileName);
		if (!fileName.empty())
		{
			if (!_saveFile("./cgi-bin/uploads/" + fileName, 
						_request.getFormObject().body, 
						_request.getFormObject().bodyLength))
			{
				_readErrorPage( STATUS_500); // check which error to return
			}
			else
				_setMsgStatusLine( STATUS_201 );
		}
		else
			_readErrorPage( STATUS_400 );
	}
}

/*
*	Handles DELETE request.
*	
*/
void	Response::_handleDelete()
{
	if (_deleteFile(_paths.responseUri) == true)
		_setMsgStatusLine( STATUS_200 );
	else {
		std::cout << GREEN << "Error deleting file" << RESET_PRINT << std::endl;
		_readErrorPage( STATUS_404 );
	}
}

/* _readErrorPage:
*	Reads the error page corresponding to the given http status code
*	and stores it in _msgBody (if existent and read-rights).
*/
void	Response::_readErrorPage( HttpStatusCode httpStatus )
{
	std::cout << RED << "_readErrorPage("<< httpStatus << ")" << RESET_PRINT << std::endl;

	_setMsgStatusLine( httpStatus );

	// Get path to error page
	std::map<HttpStatusCode, std::string>			errorPages = _config.getErrorPages();
	std::map<HttpStatusCode, std::string>::iterator	itErrorPages = errorPages.find(httpStatus);
	if ( itErrorPages == errorPages.end() ) {
		_setMsgStatusLine( STATUS_500 );
		return ;
	}
	else
		_paths.responseUri = itErrorPages->second;

	// Read error page
	if ( !_readFile( _paths.responseUri ) ) {
		if ( httpStatus != STATUS_500 ) {
			_msgStatusLine.statusCode = STATUS_500;
			_readErrorPage( _msgStatusLine.statusCode );
		}
		else {
			_msgBody = "";
			_msgBodyLength = 0;
		}
	}
	_msgHeader["Content-Length"] = to_string(_msgBodyLength);
}

/* _readFile:
*	Reads the file from the given path and stores the content in _msgBody.
*	Also stores the number of bytes read in _msgBodyLength.
*	Returns false if the file could not be opened or if the file could 
*		not be read. Otherwise true.
*/
bool	Response::_readFile( std::string path )
{
	std::ifstream	file(path.c_str(), std::ios::in | std::ios::binary);

	if ( !file.is_open() )
		return false;

	// Seek to the end of the file to determine its size
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	if ( fileSize == -1 ) {
        file.close();
		return false;
	}

	// Create a string with the size of the file and read the entire file into it
	std::string	content;
	content.resize(static_cast<size_t>(fileSize));
	if (!file.read(&content[0], fileSize)) {
		file.close();
		return false;
	}

	_msgBodyLength = fileSize;
	_msgBody = content;

	file.close();
	return true;
}

/* _saveFile:
*	Saves the given content in the file at the given path (in binary).
*	Returns false if the file could not be opened or if the file could
*		not be written. Otherwise true.
*/
bool	Response::_saveFile( std::string path, std::string content, ssize_t contentLength )
{
	std::cout << "Saving file: " << path << std::endl;
	
	if ( open(path.c_str(), O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG) == -1 )
		std::cerr << RED << "Error: could not open file with exeution rights \"" << path << "\"" << RESET_PRINT << std::endl;
	
	std::ofstream	file(path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);

	if ( !file.is_open() ) {
		std::cout << RED << "ERROR OPENING FILE" << RESET_PRINT << std::endl;
		return false;
	}
	std::cout << RED << "Content length is " << contentLength << RESET_PRINT <<  std::endl;
	// Write content to file

	if (!file.write(reinterpret_cast<const char*>(content.c_str()), contentLength)) {
		file.close();
		return false;
	}

	file.close();

	return true;
}

bool	Response::_deleteFile( std::string path )
{
	const char* filename = path.c_str();

    if (std::remove(filename) != 0)
        return false;
	else
		return true;
}

std::string Response::_buildDirectoryHtmlElement(std::string dirName, bool isDirectory)
{
	if (isDirectory == true)
    	return ("<li><a href=" + dirName + "/" + ">" + dirName + "</a></li>");
	else
    	return ("<li><a href=" + dirName + ">" + dirName + "</a></li>");
}

bool	Response::_readDir( std::string path )
{
	DIR* dir = opendir(path.c_str());
    std::string htmlBody1("<!DOCTYPE html> <html> <head> <title>List of Directories</title> </head> <body> <h1>List of Directories</h1> <ul>");
    std::string dirList("");
    std::string htmlBody2("</ul> </body> </html>");
    if (!dir) {
        return (false);
    }


    struct dirent* entry;
    while ((entry = readdir(dir))) 
	{
        // Filter out "." entries
        if (std::strcmp(entry->d_name, ".") == 0)
            continue;
        dirList += _buildDirectoryHtmlElement(entry->d_name, isDirectory(path + 
																entry->d_name));
    }
	_msgBody = htmlBody1 + dirList + htmlBody2;
	_msgBodyLength = _msgBody.size();
    closedir(dir);
	return (true);
}


/* _readHttpStatusCodeDatabase:
*	Reads the http status code database and stores it in _httpStatusCodeLookup.
*/
void	Response::_readHttpStatusCodeDatabase()
{
	std::ifstream	file( HTTP_STATS_CODE_FILE );
	std::string		line;
	std::string		status;

	if ( !file.is_open() )
	{
		std::cerr << "Error: could not open http status code database (" << HTTP_STATS_CODE_FILE << ")" << std::endl;
		return ;
	}
	while ( getline( file, line ) )
	{
		if ( line.find(',') == std::string::npos )
			continue ;
		_httpStatusCodeLookup[std::atoi(line.substr( 0, line.find(',') - 1 ).c_str())] = line.substr( line.find(',') + 1 );
	}
	file.close();
}


/**************************************************************/
/*                     GETTER METHODS                         */
/**************************************************************/

/* getResponseMsg:
*	Returns the response message as one std::string.
*/
std::string	Response::getResponseMsg() const
{
	std::string		responseMsg;

	// Status Line + Header
	responseMsg = getMsgHeader();
	
	// Body
	responseMsg.append(_msgBody, 0, _msgBodyLength);

	return responseMsg;
}

/* getMsgHeader:
*	Returns the header of the response message (inlcuding the status line) 
*	as one std::string.
*/
std::string	Response::getMsgHeader() const
{
	std::string		header;

	// Status Line
	header = _msgStatusLine.protocolVersion + " ";
	header += to_string(_msgStatusLine.statusCode) + " ";
	header += _msgStatusLine.reasonPhrase + "\r\n";

	// Header
	for ( std::map<std::string, std::string>::const_iterator it = _msgHeader.begin(); it != _msgHeader.end(); it++ )
		header += it->first + ": " + it->second + "\r\n";
	
	header += "\r\n";

	return header;
}

/* getMsgLength:
*	Returns the length of the response message.
*/
ssize_t		Response::getMsgLength() const
{
	return getMsgHeader().length() + _msgBodyLength;
}



/**************************************************************/
/*                  PROTOTYPING METHODS                       */
/**************************************************************/

/* __isCgiNeeded:
*	Checks if CGI is needed for the request.
*		
*	Returns true if CGI is needed, false otherwise.
*/
bool	Response::_isCgiNeeded()
{
	if ( (_method == GET || _method == POST) && (_request.getType() == PHP || _request.getType() == PY) )
		return true;
	else
		return false;
}

// /* FOR CGI-TESTING */
// void	ClientSocket::_buildResponseCGI( std::string path, RequestParser rp)
// {
// 	path = "";
// 	std::string output;
// 	CGIHandler CGIHandler(rp);
// 	output = CGIHandler.execute(rp);
// 	_response.body = output;
// 	std::stringstream	strStream;
// 	strStream << (_response.body.length());
	
// 	_response.header = "HTTP/1.1 200 OK\r\n";
// 	if (rp.getType() == CSS)
// 		_response.header += "Content-Type: text/css\r\n";
// 	else
// 		_response.header += "Content-Type: text/html\r\n";
// 	_response.header += "Content-Length: " + strStream.str() + "\r\n";
// 	_response.header += "\r\n";
// 	_response.contentLength = _response.header.length() + _response.body.length();
// }

// /* FOR CGI-TESTING */
// void	Response::_buildResponseCGI( std::string path )
// {
// 	path = "";
// 	std::string output;
// 	CGIHandler	CGIHandler;
// 	_msgBody = CGIHandler.execute();
// 	std::stringstream	strStream;
// 	strStream << (_msgBody.length());

// 	_msgStatusLine = "HTTP/1.1 200 OK\r\n";
// 	_msgHeader += "Content-Type: text/html\r\n";
// 	_msgHeader += "Content-Length: " + strStream.str() + "\r\n";
// 	_msgHeader += "\r\n";
// 	_msgLength = _msgStatusLine.length() + _msgHeader.length() + _msgBody.length();
// }



