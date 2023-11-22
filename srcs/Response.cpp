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

	_method = request.getMethod();
	_setPaths( request.getPath() );

	if ( !_checkPreconditions( ) )
		_readErrorPage( _msgStatusLine.statusCode );
	else if ( _checkRedirection( ) )
		;
	else if ( _method == GET )
		_handleGet();
	else if ( _method == POST )
		_handlePost();
	else if ( _method == DELETE )
		_handleDelete();

	printDebug( "+++++++++ Response Header +++++++++\n" + getMsgHeader() + "+++++++++++++++++++++++++++++++++++", 
		DEBUG_RESPONSE_HEADER, BLUE, 0 );
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
	printDebug("\tPaths:\n\t\trequestUri: " + _paths.requestUri + "\n\t\tresponseUri: " + _paths.responseUri, 
		DEBUG_PATHS, BABY_BLUE, 0);
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
*		- path/file is a directory and location allows autoindex ⇒ 404 Not Found
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
	// check if response path is a directory and the location allows autoindex
	if ((_paths.responseUri[_paths.responseUri.length() - 1] == '/') &&
		 (_config.getLocations()[_paths.confLocKey].getAutoindex() == false)) {
		printDebug("Autoindex turned off for location: " + _paths.responseUri, DEBUG_PRECOND, YELLOW, 0);
		_msgStatusLine.statusCode = STATUS_404;
		return false;
	}

	// check if path/file exists
	if ( access(_paths.responseUri.c_str(), F_OK) != 0 ) {
		printDebug("File not found: " + _paths.responseUri, DEBUG_PRECOND, YELLOW, 0);
		_msgStatusLine.statusCode = STATUS_404;
		return false;
	}

	// check if right access to path/file
	if ( (_method == GET && access(_paths.responseUri.c_str(), R_OK) != 0 ) 
		|| (_method == DELETE && access(_paths.responseUri.c_str(), X_OK) != 0 ) ) {
		printDebug("No access to file: " + _paths.responseUri, DEBUG_PRECOND, YELLOW, 0);
		_msgStatusLine.statusCode = STATUS_403;
		return false;
	}

	// check if method is allowed
	std::vector<httpMethod> methods = _config.getLocations()[_paths.confLocKey].getMethods();

	for ( std::vector<httpMethod>::iterator it = methods.begin(); it != methods.end(); it++ ) {
		if ( *it == _method )
			break ;
		else if ( it + 1 == methods.end() ) {
			printDebug("Method not allowed: " + httpMethodToString(_method), DEBUG_PRECOND, YELLOW, 0);
			_msgStatusLine.statusCode = STATUS_405;
			return false;
		}
	}

	// check if HTTP version is supported
	if ( _request.getProtocol() != _httpVersionAllowed ) {
		printDebug("HTTP version not supported: " + _request.getProtocol(), DEBUG_PRECOND, YELLOW, 0);
		_msgStatusLine.statusCode = STATUS_405;
		return false;
	}

	// check if request body size is valid
	if ( _request.getBodyLength() > _config.getClientMaxBodySize() ) {
		printDebug("Request body too large: " + to_string(_request.getBodyLength()), DEBUG_PRECOND, YELLOW, 0);
		_msgStatusLine.statusCode = STATUS_413;
		return false;
	}

	// check if Host field is present
	if ( (_request.getHost() != _config.getHost() + ":" + _config.getPort()) && 
			(_request.getHost() != _config.getServerName() + ":" + _config.getPort())) {
		printDebug("Host field not present or wrong: " + _request.getHost(), DEBUG_PRECOND, YELLOW, 0);
		_msgStatusLine.statusCode = STATUS_400;
		return false;
	}

	// check if content length is smaller 0
	if ( _request.getContentLength() < static_cast<ssize_t>(0) ) {
		printDebug("Content length smaller than 0: " + to_string(_request.getContentLength()), DEBUG_PRECOND, YELLOW, 0);
		_msgStatusLine.statusCode = STATUS_400;
		return false;
	}

	return true;
}

/* _checkRedirection:
*	Checks if the request URI is a redirection.
*		- yes ⇒ set Location field in header
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
*	Differentiate in behaviour between:
*		- autoindex: readirectory and create html page
*		- CGI call: call CGI
*		- "normal" request: read file and create response
*/
void	Response::_handleGet()
{
	if (_paths.responseUri[_paths.responseUri.length() - 1] == '/')	// Check autoindex
	{
		_readDir( _paths.responseUri );
		if ( !_readDir( _paths.responseUri ) )
			_readErrorPage( STATUS_500 );
		else
			_setMsgStatusLine( STATUS_200 );
	}
	else if ( _isCgiNeeded() ) {
		try {
			std::string cgi_folder = _config.getLocations()[_paths.confLocKey].getRoot();
			CGIHandler	CGIHandler( _request, cgi_folder );
			if ( CGIHandler.hasTimeout() )
				_readErrorPage( STATUS_504 );
			else {
				_msgBody = CGIHandler.getBody();
				_msgBodyLength = CGIHandler.getBodyLength();
				_setMsgStatusLine( STATUS_200 );
			}
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			_readErrorPage( STATUS_500 );
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
	_msgHeader["Content-Length"] = to_string(_msgBodyLength);
}

/* _handlePost:
*	Handles a POST request.
*	Differentiate in behaviour between:
*		- CGI call: call CGI
*		- "normal" request: save file (if request includes file) and create response
*			-> build error page if no file upload
*/
void	Response::_handlePost()
{
	if ( _isCgiNeeded() ) {
		try {
			// call CGI
			std::string cgi_folder = _config.getLocations()[_paths.confLocKey].getRoot();
			CGIHandler	CGIHandler( _request, cgi_folder );
			if ( CGIHandler.hasTimeout() )
				_readErrorPage( STATUS_504 );
			else {
				_msgBody = CGIHandler.getBody();
				_msgBodyLength = CGIHandler.getBodyLength();
				_setMsgStatusLine( STATUS_200 );	// check for errors!!!!
				_msgHeader["Content-Length"] = to_string(_msgBodyLength);
			}
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			_readErrorPage( STATUS_500 );
		}
	}
	else {
		std::string fileName(_request.getFormObject().fileName);
		if (!fileName.empty())
		{
			if (!_saveFile("./cgi-bin/uploads/" + fileName, 
						_request.getFormObject().body, 
						_request.getFormObject().bodyLength))
			{
				_readErrorPage( STATUS_500 );
			}
			else
				_setMsgStatusLine( STATUS_201 );
		}
		else
			_readErrorPage( STATUS_400 );
	}
}

/* _handleDelete:
*	Handles DELETE request.
*	Deletes the file in the response-URI path if it has
*	execution rights.
*/
void	Response::_handleDelete()
{
	if (_deleteFile(_paths.responseUri) == true)
		_setMsgStatusLine( STATUS_200 );
	else {
		printDebug("Error: delete file \"" + _paths.responseUri + "\" failed", 
			DEBUG_SERVER_STATE_ERROR, RED, 0);
		_readErrorPage( STATUS_404 );
	}
}

/* _readErrorPage:
*	Reads the error page corresponding to the given http status code
*	and stores it in _msgBody (if existent and read-rights).
*/
void	Response::_readErrorPage( HttpStatusCode httpStatus )
{
	printDebug("_readErrorPage(" + to_string(static_cast<ssize_t>(httpStatus)) + ")", DEBUG_SERVER_STATE_ERROR,
				RED, 0);

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

/* _isCgiNeeded:
*	Checks if CGI is needed for the request.
*		
*	Return: true if CGI is needed, false otherwise.
*/
bool	Response::_isCgiNeeded()
{
	if ( (_method == GET || _method == POST) && (_request.getType() == PHP || _request.getType() == PY) )
		return true;
	else
		return false;
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
	int	fd;
	if ( (fd = open(path.c_str(), O_RDWR|O_CREAT, S_IRWXU|S_IRWXO|S_IRWXG)) == -1 )
		printDebug("Error: could not open file \"" + path + "\" with exeution rights ", 
			DEBUG_SERVER_STATE_ERROR, RED, 0);

	std::ofstream	file(path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);

	if ( !file.is_open() ) {
		printDebug("Error: open file \"" + path + "\" failed", DEBUG_SERVER_STATE_ERROR, RED, 0);
		return false;
	}

	// Write content to file
	if (!file.write(reinterpret_cast<const char*>(content.c_str()), contentLength)) {
		file.close();
		return false;
	}

	close(fd);
	file.close();

	return true;
}

bool	Response::_deleteFile( std::string path )
{
	const char* filename = path.c_str();

    if (std::remove(filename) != 0) {
		printDebug("Error: could not delete file \"" + path + "\"", 
			DEBUG_SERVER_STATE_ERROR, RED, 0);
		return false;
	}
	else
		return true;
}

/* _buildDirectoryHtmlElement:
*	Builds a list html element for the given directory name for autoindex.
*	Return: the html element as std::string.
*/
std::string Response::_buildDirectoryHtmlElement(std::string dirName, bool isDirectory)
{
	if (isDirectory == true)
    	return ("<li><a href=" + dirName + "/" + ">" + dirName + "</a></li>");
	else
    	return ("<li><a href=" + dirName + ">" + dirName + "</a></li>");
}

/* _readDir:
*	Reads the directory at the given path and stores the content in _msgBody.
*	Return: true if the directory could be read, false otherwise.
*/
bool	Response::_readDir( std::string path )
{
	DIR* dir = opendir(path.c_str());
    std::string htmlBody1("<!DOCTYPE html> <html> <head> <title>List of Directories</title> </head> <body> <h1>List of Directories</h1> <ul>");
    std::string dirList("");
    std::string htmlBody2("</ul> </body> </html>");
    if (!dir)
        return (false);

    struct dirent* entry;
    while ((entry = readdir(dir))) {
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
*	Reads the http status code database (csv) and stores it in _httpStatusCodeLookup.
*/
void	Response::_readHttpStatusCodeDatabase()
{
	std::ifstream	file( HTTP_STATS_CODE_FILE );
	std::string		line;
	std::string		status;

	if ( !file.is_open() )
	{
		printDebug("Error: open file http status code \"" + static_cast<std::string>(HTTP_STATS_CODE_FILE) + "\" failed", 
			DEBUG_SERVER_STATE_ERROR, RED, 0);
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
