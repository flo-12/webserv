/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:31:46 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/21 14:55:03 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

/**************************************************************/
/*                 DEFAULT CON-/DESTRUCTOR                    */
/**************************************************************/

CGIHandler::CGIHandler() {}

//handle timeout for CGI, in case of endless loop

CGIHandler::CGIHandler(RequestParser rp, std::string cgi_folder): 
	_path(""), _bodyLength(0), _executable(""), _hasTimeout(false) 
{
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
    _body = _execute(rp, cgi_folder);
}

CGIHandler::~CGIHandler() {}

/**************************************************************/
/*                 GETTER & SETTER METHODS                    */
/**************************************************************/

std::string CGIHandler::getBody()
{
    return(_body);
}
ssize_t	CGIHandler::getBodyLength()
{
    return(_bodyLength);
}
bool	CGIHandler::hasTimeout() const
{
	return (_hasTimeout);
}

/**************************************************************/
/*                      PUBLIC METHODS                        */
/**************************************************************/

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
    delete[] args;
    for (unsigned long i = 0; i < _env.size(); ++i)
        delete[] env[i];
    delete[] env;
}

std::string CGIHandler::_execute(RequestParser rp, std::string cgi_folder)
{
    // creating the args
    
    _executable = "/usr/bin/php";
    if (rp.getType() != PHP)
        _executable = "/usr/bin/python3";
    _args.push_back(_executable);
    _args.push_back(cgi_folder + rp.getPath());
    
    // creating the pipes
    
    int pipeServerToCGI[2];
    int pipeCGIToServer[2];
    if (pipe(pipeServerToCGI) == -1)
        throw std::runtime_error("Error: pipe server-to-CGI failed");
    if (pipe(pipeCGIToServer) == -1)
        throw std::runtime_error("Error: pipe CGI-to-server failed");
    
    // converting the args and env for execve()
    
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

    // executing the PHP or PY in the child process
    // retrieving the output in the parent

    pid_t	pidWait = 0;
	time_t	startTime = time(0);
    std::string output;
    int pid = fork();
    if (pid == 0)
        _handleChildProcess(pipeServerToCGI, pipeCGIToServer, args, env);
	else if (pid > 0)
        _handleParentProcess(rp, pid, pipeServerToCGI, pipeCGIToServer, pidWait, startTime);
    else
    {
        _deleteArgsEnv(args, env);
        throw std::runtime_error("Error: CGI fork failed");
    }
	if ( !_hasTimeout )
		output = _readOutputChild(pipeCGIToServer);

    // cleaning the allocated memory

    _deleteArgsEnv(args, env);
    return (output);
}

void CGIHandler::_handleChildProcess(int *pipeServerToCGI, int *pipeCGIToServer, char **args, char**env)
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
    if (execve(_executable, args, env) == -1)
    {
        _deleteArgsEnv(args, env);
        throw std::runtime_error("Error: execve failed");
    }
    exit(0);
}

void CGIHandler::_handleParentProcess(RequestParser rp, int pid, int *pipeServerToCGI,
                        int *pipeCGIToServer, pid_t pidWait, time_t startTime)
{
    close(pipeServerToCGI[0]);
    close(pipeCGIToServer[1]);
    std::string body = bodyParser(rp.getBody());
	write(pipeServerToCGI[1], body.c_str(), body.length());
    close(pipeServerToCGI[1]);
    
    while (pidWait == 0 && time(0) - startTime <= TIMEOUT_CGI) {
		printDebug("Waiting for child process to terminate", DEBUG_CGI, MAGENTA, 0);
		pidWait = waitpid(pid, NULL, WNOHANG);
	}
	if (pidWait == 0) {
        printDebug("Child process still running and will be killed", DEBUG_CGI, MAGENTA, 1);
		kill(1, SIGKILL);
		_hasTimeout = true;
	}
	else if (pidWait == -1)	{ // maybe changes and error handling here????
		throw std::runtime_error("Error: waitpid failed");
	}
	else
        printDebug("Child process terminated", DEBUG_CGI, MAGENTA, 2);
}

std::string	CGIHandler::_readOutputChild( int pipeCGIToServer[2] )
{
	const int BUFSIZE = 4096;
	char buffer[BUFSIZE];
	int bytesRead = 1;
	std::string output;

	while (bytesRead > 0) {		
		bytesRead = read(pipeCGIToServer[0], buffer, BUFSIZE);
		if ( bytesRead < 0 )
			throw std::runtime_error("Error: CGI read");
		output.append(buffer, bytesRead);
		_bodyLength += bytesRead;
	}

    close(pipeCGIToServer[0]);
	return output;
}