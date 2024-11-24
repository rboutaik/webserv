#pragma once

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <stdexcept>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <ctime>
#include <map>
#include "../server/Server.hpp"

typedef struct s_eventData t_eventData;

#ifndef M_DEBUG
# define M_DEBUG 0
#endif

#define CLIENT_TIMEOUT_SEC 30
#define CGI_TIMEOUT_SEC 10

class KQueue
{

public:
    static int createKq();
    static void closeKq();
    static int getFd() {return m_fd;}
    static int watchState(int fd, t_eventData* evData, int type);
    static void removeWatch(int fd, int type);
    static int getEvents(struct kevent* buffArray, int size, int& watchedStates);
    static void setFdNonBlock(int fd);
    static void waitForClientToSend(int clientSock, std::vector<VirtualServer>* s);
    static int watchChildExited(pid_t pid, t_eventData* evData);
    
    static struct kevent m_keventBuff;
    static struct timespec m_timout;
    static std::map<t_eventData*, std::time_t> connectedClients;
    static std::map<t_eventData*, std::time_t> startedCgis;

private:
    static int m_fd;

};