/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:31:46 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/01 09:44:00 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler() {}

CGIHandler::CGIHandler(RequestParser rp): _path("") 
{
    rp.getBody();
    std::cout << std::endl << "----------////#####" << std::endl;
    _env.push_back("CONTENT_LENGTH=17");
}

CGIHandler::~CGIHandler() {}

std::string CGIHandler::execute()
{
    int pipefd[2];
    _args.push_back("php");
    _args.push_back("/home/fbecht/42_Projects/Level_5/webserv/cgi-bin/hello_world.php");
    pipe(pipefd);
    char **args = new char*[_args.size() + 1];
    for (unsigned long i = 0; i < _args.size(); ++i)
    {
        args[i] = new char[_args[i].length() + 1];
        std::strcpy(args[i], _args[i].c_str());
    }
    args[_args.size()] = NULL;
    char **env = new char*[_env.size() + 1];
    for (unsigned long i = 0; i < _env.size(); ++i)
    {
        env[i] = new char[_env[i].length() + 1];
        std::strcpy(env[i], _env[i].c_str());
    }
    env[_env.size()] = NULL;
    std::string output;
    int pid = fork();
    if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        execve("/usr/bin/php", args, env);
        perror("execve");
        exit(0);
    }
    else if (pid > 0)
    {
        const int BUFSIZE = 4096;
        char buffer[BUFSIZE] = {0};
        int bytesRead = 1;
        waitpid(pid, NULL, 0);
        close(pipefd[1]);
        while (bytesRead > 0) 
        {
            bytesRead = read(pipefd[0], buffer, BUFSIZE);
            output.append(buffer, bytesRead);
        }
        //std::cout << output << std::endl;
        close(pipefd[0]);
    }
    else
        throw std::runtime_error("Error: CGI fork failed");
    return (output);
}