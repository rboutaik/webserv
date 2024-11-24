#pragma once

class HttpRequest;
class HttpResponse;
class   VirtualServer;
#include "../HttpProtocol/Request.hpp"
#include "../HttpProtocol/Response.hpp"
#include "../server/Server.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct s_sockData {
    sockaddr_in* sockAddress;
    socklen_t* sockLen;
    s_sockData(): sockAddress (NULL), sockLen(NULL) {}
} t_sockData;

typedef struct s_eventData {
    const char* type;
    HttpResponse* resData;
    HttpRequest* reqData;
    t_sockData* serverData;
    std::vector<VirtualServer> *s;
    s_eventData(const char* type, HttpResponse* data): type(type), resData(data), reqData(NULL), serverData(NULL), s() {}
    s_eventData(const char* type, HttpRequest* data): type(type), resData(NULL), reqData(data), serverData(NULL), s() {}
    s_eventData(const char* type, t_sockData* data): type(type), resData(NULL), reqData(NULL), serverData(data), s() {}
    ~s_eventData() throw();
} t_eventData;
