/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:31:46 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/14 07:50:25 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler() {}

//handle timeout for CGI, in case of endless loop

CGIHandler::CGIHandler(RequestParser rp): _path("") 
{
    //std::cout << std::endl << "----------////#####" << std::endl;
    // const int timeout = 5;
    // clock_t start = clock();
    
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
    _env.push_back("SCRIPT_FILENAME=" + rp.getPath());;
    _body = _execute(rp);
    // while (true)
    // {
    //     clock_t current = clock();
    //     int elapsedSeconds = static_cast<double>(current - start) / CLOCKS_PER_SEC;
        
    //     if (elapsedSeconds >= timeout)
    //     {
    //         std::cout << "Timeout" << std::endl;
    //         break ;
    //     }
    // } //when execve can't find the executable
}

CGIHandler::~CGIHandler() {}

std::string CGIHandler::getBody()
{
    return(_body);
}

ssize_t CGIHandler::getBodyLength()
{
    return(_bodyLength);
}

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

void CGIHandler::_deleteArgsEnv(char **args, char **env)
{
    for (unsigned long i = 0; i < _args.size(); ++i)
        delete[] args[i];
    for (unsigned long i = 0; i < _env.size(); ++i)
        delete[] env[i];
}

std::string CGIHandler::_execute(RequestParser rp)
{
    const char *executable = "/usr/bin/php";
    if (rp.getType() != PHP)
    {
        executable = "/usr/bin/python3";
    }
    _args.push_back(executable);
    _args.push_back("./cgi-bin" + rp.getPath());
    
    int pipeServerToCGI[2];
    int pipeCGIToServer[2];
    if (pipe(pipeServerToCGI) == -1)
        throw std::runtime_error("Error: pipe server-to-CGI failed");
    if (pipe(pipeCGIToServer) == -1)
        throw std::runtime_error("Error: pipe CGI-to-server failed");
    
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
    
    // if (access(args[1], F_OK) == 0)
    //     std::cout << "file detected" << std::endl;
    
    std::string output;
    int pid = fork();
    if (pid == 0)
    {
        close(pipeServerToCGI[1]);
        close(pipeCGIToServer[0]);
        if (dup2(pipeServerToCGI[0], STDIN_FILENO) == -1)
        {
            _deleteArgsEnv(args, env);
            throw std::runtime_error("Error: dup2 server-to-CGI in child process failed");
        }
        if (dup2(pipeCGIToServer[1], STDOUT_FILENO) == -1)
        {
            _deleteArgsEnv(args, env);
            throw std::runtime_error("Error: dup2 CGI-to-server in child process failed");
        }
        if (execve(executable, args, env) == -1)
        {
            _deleteArgsEnv(args, env);
            throw std::runtime_error("Error: execve failed");
        }
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
            output.append(buffer, bytesRead);
        }
        waitpid(pid, NULL, 0);
    }
    else
    {
        _deleteArgsEnv(args, env);
        throw std::runtime_error("Error: CGI fork failed");
    }
    _bodyLength = output.length();
    _deleteArgsEnv(args, env);
    std::cout << "output is: " << output << std::endl;
    return (output);
}