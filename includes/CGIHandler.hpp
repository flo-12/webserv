/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:31:36 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/21 14:23:15 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP

# define CGIHANDLER_HPP

# include <iostream>
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <RequestParser.hpp>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <cstring>
# include <cstdlib>
# include <sstream>
# include <string>
# include <common.hpp>
# include <ctime>

class CGIHandler
{
    public:
            CGIHandler();
            CGIHandler(RequestParser rp, std::string cgi_folder);
            ~CGIHandler();

            std::string getBody();
            ssize_t getBodyLength();
			bool		hasTimeout() const;
            
    private:
            std::string _execute(RequestParser rp, std::string cgi_folder);
            std::string _path;
            std::vector<std::string> _args;
            std::vector<std::string> _env;
            std::string _body;
            ssize_t     _bodyLength;
            const char *_executable;
			bool		_hasTimeout;
            void        _handleChildProcess(int *pipeServerToCGI, 
                            int *pipeCGIToServer, char **args, char**env);
            void        _handleParentProcess(RequestParser rp, int pid, int *pipeServerToCGI, 
                            int *pipeCGIToServer, pid_t pidWait, time_t startTime);

            void        _deleteArgsEnv(char **args, char **env);
            std::string bodyParser(std::string requestBody);
			std::string	_readOutputChild( int pipeCGIToServer[2]);
};

#endif