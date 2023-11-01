/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 12:35:40 by pdelanno          #+#    #+#             */
/*   Updated: 2023/10/31 10:07:07 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSER_HPP

# define REQUESTPARSER_HPP

# include <iostream>
# include <sstream>
# include <map>
# include <vector>

class RequestParser
{
    public:
            RequestParser();
            RequestParser(std::string buffer);
            ~RequestParser();
            void parseRequest(std::string const &buffer);
            std::string getMethod();
            std::string getPath();
            std::string getQuery();
            std::string getProtocol();
            std::string getBody();
            std::map<std::string, std::string> getHeaders();

            std::string removeCarriageReturn(std::string &str);

    private:
            std::string _method;
            std::string _path;
            std::string _query;
            std::string _protocol;
            std::map<std::string, std::string> _headers;
            std::string _body;

            void parseHeaders();
};

// class Requests
// {
//     public:
//             Requests();
//             ~Requests();
            
//             void parseRequests(std::string const &buffer);
//     private:
//             std::vector<RequestParser> _requests;
//             int _requestCount;
// };

std::ostream &operator<<(std::ostream &str, RequestParser &rp);
std::ostream &operator<<(std::ostream &str, std::map<std::string, std::string> &m);

#endif