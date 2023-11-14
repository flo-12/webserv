/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdelanno <pdelanno@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 12:35:40 by pdelanno          #+#    #+#             */
/*   Updated: 2023/11/14 18:42:53 by pdelanno         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSER_HPP

# define REQUESTPARSER_HPP

# include "common.hpp"
# include <iostream>
# include <string>
# include <sstream>
# include <map>
# include <vector>
# include <sys/types.h>

class RequestParser
{
    private:
            httpMethod  _method;
            std::string _path;
            std::string _query;
            fileExtension _type;
            std::string _protocol;
            std::map<std::string, std::string> _headers;
            std::string _host;
            ssize_t     _contentLength;
            std::string _body;
            ssize_t     _bodyLength;
            std::string _contentType;
            //std::string _formData;
            bool _isUpload;
            std::string _boundaryCode;

            class formObject
            {
                public:
                        formObject() {};
                        ~formObject() {};
                        std::string name;
                        std::string fileName;
                        std::string formBody;
            };
            
            formObject _form;
            void _handleBoundaries(std::string boundary);
            
    public:
            RequestParser();
            RequestParser(std::string buffer, ssize_t contentLength);
            ~RequestParser();
            void        parseRequest(std::string const &buffer);
            httpMethod  getMethod() const;
            std::string getPath() const;
            std::string getQuery() const;
            fileExtension getType() const;
            std::string getProtocol() const;
            std::string getBody() const;
            std::string getHost() const;
            std::map<std::string, std::string> getHeaders() const;
            ssize_t     getContentLength() const;
            formObject  getFormObject() const;

            std::string removeCarriageReturn(std::string &str);

};
std::ostream &operator<<(std::ostream &str, RequestParser &rp);
std::ostream &operator<<(std::ostream &str, std::map<std::string, std::string> &m);

#endif