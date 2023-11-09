#pragma once

#include "common.hpp"
#include "CGIHandler.hpp"
#include "ServerConfig.hpp"
#include "RequestParser.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>


typedef struct s_StatusLine
{
	std::string		protocolVersion;
	HttpStatusCode	statusCode;
	std::string		reasonPhrase;
} t_StatusLine;

typedef struct s_paths
{
	std::string	requestUri;
	std::string	confLocKey;
	std::string	responseUri;	// /html/index.html
} t_paths;



class Response
{
	private:
		httpMethod							_method;
		RequestParser						_request;		
		ServerConfig						_config;
		std::string							_httpVersionAllowed;

		t_paths								_paths;
		std::map<int, std::string>			_httpStatusCodeLookup;

		t_StatusLine						_msgStatusLine;
		std::map<std::string,std::string>	_msgHeader;
			// Content-Length
			// Location
		std::string							_msgBody;
		ssize_t								_msgBodyLength;

		void	_readHttpStatusCodeDatabase();
		void	_setPaths( std::string reqUri );
		void	_setMsgStatusLine( HttpStatusCode httpStatus );

		bool	_checkPreconditions();
		bool	_checkRedirection();

		void	_handleGet();
		void	_readErrorPage( HttpStatusCode httpStatus );
		bool	_readFile( std::string path );

		// Prototyping
		// void		_buildResponse( std::string path );
		// void		_buildResponseCGI( std::string path );
		// void		_saveFile( std::string path, std::string content );		

	public:
		Response( HttpStatusCode httpStatus, ServerConfig config );
		Response( RequestParser request, ServerConfig config );
		~Response();
	
		// Getters
		std::string	getResponseMsg() const;
		std::string	getMsgHeader() const;
		ssize_t		getMsgLength() const;

} ;