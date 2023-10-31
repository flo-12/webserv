/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 11:12:49 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/31 11:12:51 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Socket.hpp"

/* Class ServerSocket:
*	- inherits from Socket
*	- default constructor sets _type to SERVER
*	- represents a server socket
*	- extends the Socket with the server's specific methods and data; E.g.:
*		- ip adress and port number
*/
class ServerSocket : public Socket
{
	private:
		unsigned int	_ip;
		int				_port;
		int				_nbrRestarts;
	
		void	setupSocket();

	public:
		ServerSocket( unsigned int ip, int port );
		~ServerSocket();

		void	restartServerSocket();
} ;
