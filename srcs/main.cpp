/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbecht <fbecht@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/23 18:03:08 by fbecht            #+#    #+#             */
/*   Updated: 2023/10/23 18:03:13 by fbecht           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include <cstdlib>	// for EXIT_SUCCESS and EXIT_FAILURE

int	main( int argc, char **argv )
{
	if (argc == 255)
		return EXIT_FAILURE;
	(void)argv;
	/* if ( argc == 1 )		// default confid-File
	{}
	else if ( argc == 2 )	// config-File at argv[1]
	{}
	else					// wrong usage
	{} */

	
	//signal(SIGINT, signal_handler);
	
	//try
	//{
		WebServ webserv;

		webserv.serverRun();
	/* }
	catch( const std::exception& e )
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	} */
	
	return EXIT_SUCCESS;
}
