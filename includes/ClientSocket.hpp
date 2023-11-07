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
//#include "CGIHandler.hpp"
#include "Response.hpp"
#include <ctime>


typedef struct s_reqStatus {
	time_t		startTime;
	bool		pendingReceive;
	ssize_t		contentLength;
	ssize_t		readBytes;
	std::string	buffer;
} t_reqStatus;

typedef struct s_resStatus {
	time_t		startTime;
	bool		startSending;
	bool		pendingSend;
	ssize_t		msgLength;
	ssize_t		sentBytes;
	std::string	message;
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
	
	public:
		ClientSocket( int serverFd, ServerConfig config );
		~ClientSocket();

		// Socket methods
		void			setupSocket();
		void			closeConnection( HttpStatusCode httpStatus );
		bool			hasTimeout();
		ReceiveStatus	receiveRequest();
		ResponseStatus	sendResponse();
} ;
