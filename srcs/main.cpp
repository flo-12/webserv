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

#include "../includes/common.hpp"
#include "../includes/ConfigParser.hpp"
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
	std::string		configFile;

	if ( argc == 1 )		// default config-File
		configFile = "configs/default.conf";
	else if ( argc == 2 )	// config-File at argv[1]
		configFile = argv[1];
	else					// wrong usage
	{
		std::cerr << "Usage: " << argv[0] << " [config-file]" << std::endl;
		return EXIT_FAILURE;
	}

	if ( signal(SIGINT, sigint_handler) == SIG_ERR )
	{
		std::cerr << "Error setting up signal handler" << std::endl;
		return EXIT_FAILURE;
	}
	
	try
	{
		ConfigParser	config( configFile );

		WebServ webserv( config.getIpPort() );
		webservPtr = &webserv;

		webserv.serverRun();
	}
	catch( const std::exception& e )
	{
		std::cerr << e.what() << '\n';
		if ( webservPtr )
			webservPtr->serverShutdown();
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
