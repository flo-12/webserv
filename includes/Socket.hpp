/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/27 16:07:37 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/27 16:07:39 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <unistd.h>


typedef enum eSocketType
{
	UNUSED,
	SERVER,
	CLIENT
} SocketType;

typedef struct s_reqStatus {
	bool		pendingReceive;
	ssize_t		contentLength;
	ssize_t		readBytes;
	std::string	buffer;
} t_reqStatus;

typedef struct s_resStatus {
	bool		pendingSend;
	ssize_t		contentLength;
	ssize_t		sentBytes;
	std::string	header;
	std::string	body;
} t_resStatus;

class Socket
{
	friend class WebServ;

	private:
		unsigned int	_ip;
		int				_port;
		int				_fd;
		SocketType		_type;
		t_reqStatus		_request;
		t_resStatus		_response;


	public:
		Socket( SocketType type );
		~Socket();

		//void		createRequest( std::string request );
		void		buildResponse();

		bool	operator==( const Socket& other ) const;
};
