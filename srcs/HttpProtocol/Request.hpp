#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <algorithm>
#include "Exceptions.hpp"

class VirtualServer;
std::string  generateRandomFileName(const std::string path, const std::string extension);


enum ParseState{
    FirstLine,
    Headers,
    Body
};

class HttpRequest
{

public:
    HttpRequest();
    void readRequest(int fd);
    void performChecks(void);
    void parseRequest(const std::string& line);
    void parseFirstLine(std::string line);
    void parseHeaders(std::string line);
    void parseBody(char *line, size_t size);
    void generateUniqueFile(void);
    void unchunkBody(char *request, size_t size);
    bool isDone;
    std::string getHeader(const std::string& key){
        return headers[key];
    };
    long content_length; 
    long total_read_bytes;

    std::string method ,uri, version, boundary, bodyFile;
    double chunk_size, bodyRead;
    std::map<std::string, std::string> headers;
    std::vector<char> partial_data;
    ParseState state;
    ssize_t read_bytes;
    long long chunkPos;
    std::string TransferEncoding;
    bool skipNextLine;
    bool IsCgi;
    std::vector<VirtualServer>* s;
    VirtualServer *mainServ;
    int clientSocket;
    pid_t cgiPid;

};


#endif
