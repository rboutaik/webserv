#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <sstream>
#include "Request.hpp"
#include "Exceptions.hpp"
#include "../KQueue/KQueue.hpp"

std::string WhatContentType(std::string uri);
std::string strToLower(std::string s);

#ifndef M_DEBUG
# define M_DEBUG 0
#endif

class VirtualServer;

class HttpResponse{
    public :
        std::string Version;
        std::string ResponseCode;
        std::string ContentType;
        std::string Connection;
        std::vector<char> Body;
        int clientSocket;
        int responseFd;
        bool ended;
        size_t iterations;
        std::vector<VirtualServer>* s;
        bool connectionClose;
        pid_t cgiPid;
        std::string reqUri;
        bool reqIsCgi;


        HttpResponse(int clientSocket, int fd, HttpRequest* req) : Version("HTTP/1.1"), ResponseCode("200 OK"), ContentType(WhatContentType(req->uri)), Connection("close"), clientSocket(clientSocket), responseFd(fd), ended(false) {

            reqUri = req->uri;
            reqIsCgi = req->IsCgi;
            iterations = 0;
            s = req->s;
            connectionClose = (strToLower(req->getHeader("Connection")) == "close\r\n");
            cgiPid = req->cgiPid;
        }
        ~HttpResponse() {close(responseFd);}
        void sendingResponse(long buffSize);
        void sendHeaders();

        std::string GetVersion();
        std::string GetResponseCode();
        std::string GetContentType();
        std::string GetConnection();
        const std::vector<char>& GetBody();
        const std::vector<char> BuildResponse();

};

