# Webserv - 42 Project

## Description

**Webserv** is a custom-built HTTP server developed as part of the 42 curriculum. The server is designed to handle basic HTTP requests, serve static files, and process CGI requests. It follows the HTTP/1.1 protocol and allows serving multiple virtual hosts. The project focuses on network programming, multi-threading, and HTTP protocol understanding.

## Features

- **HTTP/1.1 Support**: Handle basic GET and POST requests.
- **Static File Serving**: Serve `.html`, `.css`, `.js` files, etc.
- **CGI Support**: Ability to execute CGI scripts for dynamic content.
- **Virtual Hosts**: Support multiple websites from a single server instance.
- **Custom Configuration**: Configure server behavior via a configuration file.
- **Error Handling**: Graceful handling of HTTP errors (e.g., 404, 500).

## Installation

### Clone the Repository

```bash
git clone https://github.com/rboutaik/webserv.git
cd webserv
./webserv path_to_config_file
For example:

./webserv ./config/webserv.conf
The server will start listening on the configured port and serve files based on the configuration file.

Configuration
The server's behavior is controlled by a configuration file (e.g., webserv.conf).
```
