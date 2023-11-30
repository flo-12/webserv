# Webserv

This project is about writing our own HTTP server in C++. HTTP or **Hypertext Transfer Protocol** is the foundation of data communication for the World Wide Web, where hypertext documents include hyperlinks to other resources that the user can easily access. The primary function of a web server is to store, process, and deliver web pages to clients. The communication between client and server takes place using HTTP requests, such as GET, POST or DELETE to name just a few.
A user agent, commonly a web browser, initiates communication by requesting a specific resource using HTTP and the server responds with the content of that resoruce or an error message if unable to do so (i.e. Error 404 for when the resource doesn't exist).
While the primary function is to serve content, full implementation of HTTP also includes ways of receiving content from clients. This feature is used for submitting web forms, such as the uploading of files.

Here is an example of what happens after launching our webserv and entering `http://127.0.0.1:18000` in Firefox:

![webserv1](https://github.com/flo-12/webserv/assets/104844198/9c1f68dc-f9a8-4955-9091-52c420f6d52d)

In the console (`F12`), we can see the 4 GET requests sent by the browser to the server:

![webserv2](https://github.com/flo-12/webserv/assets/104844198/814d6989-2e4b-4a12-9464-1e3519c83e6a)

And the response from the server (HTML, CSS, and JS):

![webserv3](https://github.com/flo-12/webserv/assets/104844198/ed59562e-44a0-4e07-b209-092bb264ed82)

## Usage

Warning: `Makefile` is configured for `Linux` use only.

> - Compile with `make`
> - Then run with `./webserv` + (optional) `CONFIGFILE` | Example: `./webserv myserver.conf`
> - Running the program with no argument will load `default.conf` by default
> - Access the server through a browser at address `http://127.0.0.1:18000`
> - To shut the server down, use `Ctrl` + `C`

## Basic Concepts

# CGI

CGI stands for **Common Gateway Interface**. Thanks to this interface, a web server can execute an external
program and return the result of that execution. For example, if you click on the CGI button of our web page,
you'll see that the link takes you too `http://127.0.0.1:18000/index.php`. HTTP does not handle `.php` files
by default, and this is why we need a CGI in that case.
In order to execute a PHP file, we need the **PHP executable** usually located at `/usr/bin/php` on Linux.
The job of the CGI is to execute the PHP file and retrieve the result of that execution. 
In our case, we use PHP to execute a C++ program called RPN (Reverse Polish Notation, mathematical notation in
which operators follow their operands, ex: `4 4 5 + +`). After entering an operation in the submit field, a POST
request is sent to the server. The content of the field is passed as argument to the RPN program, and the result
of the execution gets embedded in an HTML body thanks to PHP.







