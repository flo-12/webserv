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
#include <csignal>	// for signal()

WebServ*	webservPtr = NULL;

// Signal handler function for SIGINT (CTRL+C)
void	sigint_handler( int signum )
{
	(void)signum;
	std::cout << std::endl << "Received CTRL+C (SIGINT) signal. Exiting..." << std::endl;
	if ( webservPtr )
		webservPtr->serverShutdown();
	std::exit(EXIT_SUCCESS);
}

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
	
	if ( signal(SIGINT, sigint_handler) == SIG_ERR )
	{
		std::cerr << "Error setting up signal handler" << std::endl;
		return EXIT_FAILURE;
	}
	
	try
	{
		WebServ webserv;
		webservPtr = &webserv;

		webserv.serverRun();
	}
	catch( const std::exception& e )
	{
		std::cerr << e.what() << '\n';
		webserv.serverShutdown();
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
