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


typedef enum eSocketType
{
	UNUSED,
	SERVER,
	CLIENT
} SocketType;

class Socket
{
	friend class WebServ;

	private:
		unsigned int	_ip;
		int				_port;
		int				_fd;
		SocketType		_type;


	public:
		Socket( int fd );
		~Socket();

		void		createRequest( std::string request );
		std::string	buildResponse();
};
