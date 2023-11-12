/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:31:36 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/12 15:53:35 by pdelanno         ###   ########.fr       */
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
            CGIHandler(RequestParser rp);
            ~CGIHandler();

            std::string getBody();
            ssize_t getBodyLength();
            
    private:
            std::string _execute(RequestParser rp);
            std::string _path;
            std::vector<std::string> _args;
            std::vector<std::string> _env;
            std::string _body;
            ssize_t     _bodyLength;
            void        _deleteArgsEnv(char **args, char **env);

            std::string bodyParser(std::string requestBody);
};

#endif