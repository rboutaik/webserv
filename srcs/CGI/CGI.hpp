#pragma once

#include <iostream>
#include <unistd.h>
#include "../KQueue/KQueue.hpp"
#include "../HttpProtocol/Request.hpp"
#include "../Methods/Methods.hpp"

class HttpRequest;
#ifndef M_DEBUG
# define M_DEBUG 0
#endif

#ifndef BUFF_SIZE
# define BUFF_SIZE 64000
#endif

#define NOCHILD -2

extern char **environ;

class CGI {

public:
    static char** setupCGIEnvironment(HttpRequest* req);
    static int responseCGI(HttpRequest* req, int BodyFd, Location *location);

};