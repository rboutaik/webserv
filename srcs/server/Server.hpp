
#pragma once

#include <iostream>
#include <climits>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <map>
#include <unistd.h>

#include "../configFile/Directive.hpp"
#include "../configFile/Location.hpp"

#include "../HttpProtocol/Request.hpp"
#include "../HttpProtocol/Response.hpp"
#include "structs.hpp"
#include "../KQueue/KQueue.hpp"


class   VirtualServer
{
    public:
        std::map<std::string, Directive> directives;
        std::map<std::string, Location> locations;
        // std::map<std::string, std::time_t> session_ids;
        std::string server_name;
};


class   Server
{

public:
    Server();
    ~Server();
    void init();
    int getSocket() const {return m_socket;}

    sockaddr_in m_sockAddress;
    socklen_t m_sockLen;
    struct s_sockData m_sockData;
    struct s_eventData m_sEventData;

    std::vector<VirtualServer>   serv;

private:
    int m_socket;

};

