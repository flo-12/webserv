/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:31:46 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/10 11:27:14 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler()
{
    std::cout << std::endl << "----------////#####" << std::endl;
    _env.push_back("CONTENT_LENGTH=9");
    _env.push_back("REQUEST_METHOD=POST");
    _env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
    _env.push_back("SCRIPT_FILENAME=/home/pdelanno/webserv-group/cgi-bin/RPN.php");
}

//handle timeout for CGI, in case of endless loop

CGIHandler::CGIHandler(RequestParser rp): _path("") 
{
    rp.getBody();
    std::cout << std::endl << "----------////#####" << std::endl;
    
    _env.push_back("CONTENT_LENGTH=" + rp.getHeaders()["Content-Length"]);
    std::string type;
    if (rp.getMethod() == GET)
        type = "GET";
    else if (rp.getMethod() == POST)
        type = "POST";
    else
        type = "DELETE";
    _env.push_back("REQUEST_METHOD=" + type);
    _env.push_back("CONTENT_TYPE=" + rp.getHeaders()["Content-Type"]);
    _env.push_back("SCRIPT_FILENAME=" + rp.getPath());
    //std::cout << _env[3] << std::endl;
}

CGIHandler::~CGIHandler() {}

std::string CGIHandler::bodyParser(std::string requestBody)
{
    requestBody.erase(0, 7);
    std::string body = "\"";
    char c;
    std::istringstream linestream(requestBody);
    while (linestream >> c)
    {
        if (c == '+')
            body += " ";
        else if (c == '%')
        {
            char c1, c2;
            std::string hexa = "";
            linestream >> c1 >> c2; hexa = c1; hexa += c2;
            std::istringstream hexaValue(hexa);
            int intValue;
            hexaValue >> std::hex >> intValue;
            body += static_cast<char>(intValue);
        }
        else
            body += c;
    }
    body += "\"";
    return(body);
}

std::string CGIHandler::execute(RequestParser rp)
{
    const char *executable = "/usr/bin/php";
    if (rp.getType() != PHP)
    {
        executable = "/usr/bin/python3";
    }
    _args.push_back("/usr/bin/php");
    _args.push_back("./cgi-bin" + rp.getPath());
    int pipeServerToCGI[2];
    int pipeCGIToServer[2];
    pipe(pipeServerToCGI);
    pipe(pipeCGIToServer);
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
    std::cout << _env[1] << std::endl;
    std::string output;
    int pid = fork();
    if (pid == 0)
    {
        close(pipeServerToCGI[1]);
        close(pipeCGIToServer[0]);
        dup2(pipeServerToCGI[0], STDIN_FILENO);
        dup2(pipeCGIToServer[1], STDOUT_FILENO);
        execve(executable, args, env);
        perror("execve");
        exit(0);
    }
    else if (pid > 0)
    {
        close(pipeServerToCGI[0]);
        close(pipeCGIToServer[1]);
        std::string body = bodyParser(rp.getBody());
        write(pipeServerToCGI[1], body.c_str(), body.length());
        close(pipeServerToCGI[1]);
        const int BUFSIZE = 4096;
        char buffer[BUFSIZE];
        int bytesRead = 1;
        while (bytesRead > 0) 
        {
            bytesRead = read(pipeCGIToServer[0], buffer, BUFSIZE);
            //std::cout << buffer << std::endl;
            output.append(buffer, bytesRead);
        }
        waitpid(pid, NULL, 0);
        //std::cout << output << std::endl;
    }
    else
        throw std::runtime_error("Error: CGI fork failed");
    return (output);
}