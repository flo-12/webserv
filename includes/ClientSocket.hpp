/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 11:12:39 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/31 11:12:41 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "common.hpp"
#include "Socket.hpp"
#include "RequestParser.hpp"
#include "CGIHandler.hpp"
#include <ctime>

//# define MAX_REQ_SIZE 8192
//# define TIMEOUT_RECEIVE 8192

typedef struct s_reqStatus {
	time_t		startTime;
	bool		pendingReceive;
	ssize_t		contentLength;
	ssize_t		readBytes;
	std::string	buffer;
} t_reqStatus;

typedef struct s_resStatus {
	time_t		startTime;
	bool		pendingSend;
	ssize_t		contentLength;
	ssize_t		sentBytes;
	std::string	header;
	std::string	body;
} t_resStatus;

typedef enum e_ReceiveStatus
{
	READ_ERROR = -1,
	READ_PENDING,
	READ_DONE,
	CLIENT_CLOSED
} ReceiveStatus;

typedef enum e_ResponseStatus
{
	SEND_ERROR = -1,
	SEND_PENDING,
	SEND_DONE
} ResponseStatus;

typedef enum e_HttpErrorType
{
	NO_ERROR = -1,
	ERROR_DEFAULT,
	ERROR_400,
	ERROR_403,
	ERROR_404,
	ERROR_405,
	ERROR_408,
	ERROR_500,
	ERROR_505
} HttpErrorType;


class ClientSocket : public Socket
{
	private:
		t_reqStatus		_request;
		t_resStatus		_response;
		int				_serverFd;
		time_t			_startTime;

		void	_clearRequest();
		void	_clearResponse();

		bool	_requestComplete();

		// Response Parser
		void	_buildResponse( std::string path );
		void	_buildResponseCGI( std::string path );
		std::string	_readFile( std::string path );
		void	_saveFile( std::string path, std::string content );
	
	public:
		ClientSocket( int serverFd );
		~ClientSocket();

		// Socket methods
		void			setupSocket();
		void			closeConnection( HttpErrorType httpError );
		bool			hasTimeout();
		ReceiveStatus	receiveRequest();
		ResponseStatus	sendResponse();
} ;
