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


typedef enum eSocketType
{
	UNUSED,
	SERVER,
	CLIENT
} SocketType;

typedef struct s_reqStatus {
	bool		pendingReceive;
	int			contentLength;	// for POST
	int			readBytes;
	std::string	buffer;
} t_reqStatus;

class Socket
{
	friend class WebServ;

	private:
		unsigned int	_ip;
		int				_port;
		int				_fd;
		SocketType		_type;
		t_reqStatus		_reqStatus;


	public:
		Socket( int fd );
		~Socket();

		void		createRequest( std::string request );
		std::string	buildResponse();
};
