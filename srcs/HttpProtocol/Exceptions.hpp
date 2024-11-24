#pragma once


#include <iostream>
#include <exception>
#include <unistd.h>
#include <fstream>
#include <sys/socket.h>
#include <vector>
#include "../configFile/Directive.hpp"

#ifndef M_DEBUG
# define M_DEBUG 0
#endif

class ErrorStatus;
class SuccessStatus;

class ErrorStatus {
public:
    // You can Pass NUll to debugMsg
    ErrorStatus(int clienSocket, int errorCode, const char* debugMsg);
    ErrorStatus(int errorCode, const char* debugMsg);
    ErrorStatus(int errorCode, const char* debugMsg, Directive *errorpages);
    void setErrorMessage();
    ~ErrorStatus() {
        if (clientSock != -1) {
            close(clientSock);
            M_DEBUG && std::cerr << "Closed connection after sending error response\n";
        }
    }
    void sendError() const throw() {
        if (clientSock != -1 && errorCode != -1) {
            int s = send(clientSock, statusMessage.c_str(), statusMessage.size(), 0);
            if (s == -1) {
                if (M_DEBUG) perror("send(2)");
            } else if (s == 0)
                M_DEBUG && std::cerr << "User closed connection\n";
        }
    }
    const char* what() const throw() {return (statusMessage.c_str());}
    int clientSock;
    int errorCode;
    const std::string headers;
    std::string statusMessage;
    Directive *errorpages;
};

class SuccessStatus {
public:
    // You can Pass NUll to debugMsg
    SuccessStatus(int clienSocket, int successCode, const char* debugMsg, bool connClose);
    SuccessStatus(int successCode, const char* debugMsg, bool connClose);
    SuccessStatus(int successCode, const char* debugMsg, std::string retur, bool connClose);
    void setSuccessMessage();
    ~SuccessStatus() {
        if (clientSock != -1) {
            if (connClose) {
                close(clientSock);
                M_DEBUG && std::cerr << "Closed connection after sending success response\n";
            }
        }
    }
    void sendSuccess() {
        if (clientSock != -1 && successCode != -1) {
            int s = send(clientSock, statusMessage.c_str(), statusMessage.size(), 0);
            if (s == 0 || s == -1)
                connClose = true;
        }
    }
    const char* what() const throw() {return (statusMessage.c_str());}
    int clientSock;
    int successCode;
    const std::string headers;
    std::string statusMessage;
    std::string retur;
    bool connClose;
};
