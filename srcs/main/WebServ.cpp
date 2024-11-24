#include "WebServ.hpp"

bool    cgiPathValid(Location *location, std::string extension)
{
    if ((!strcmp(extension.c_str(), ".php") || !strncmp(extension.c_str(), ".php?", 5)) && (location->directives.find("php-cgi") == location->directives.end() || access(location->directives["php-cgi"].values[0].c_str(), F_OK) != 0))
        return (0);
    if ((!strcmp(extension.c_str(), ".py") || !strncmp(extension.c_str(), ".py?", 5)) && (location->directives.find("py-cgi") == location->directives.end() || access(location->directives["py-cgi"].values[0].c_str(), F_OK) != 0))
        return (0);
    return (1);
}


std::string decode_url(const std::string encoded_url) {
    std::string decoded_url;
    for (size_t i = 0; i < encoded_url.length(); ++i) {
        if (encoded_url[i] == '%' && i + 2 < encoded_url.length() &&
            std::isxdigit(encoded_url[i + 1]) && std::isxdigit(encoded_url[i + 2])) {
            // Extract the hex code and convert to character
            std::string hex_value = encoded_url.substr(i + 1, 2);
            char decoded_char = static_cast<char>(std::strtol(hex_value.c_str(), NULL, 16));
            decoded_url += decoded_char;
            i += 2;  // Skip next two characters (they are part of the encoded value)
        } else if (encoded_url[i] == '+') {
            // Convert '+' to space (' ')
            decoded_url += ' ';
        } else {
            // Copy non-encoded characters directly
            decoded_url += encoded_url[i];
        }
    }
    return decoded_url;
}

static int checkAndOpen(HttpRequest* req)
{
    int body_fd;
    int fd;

    M_DEBUG && std::cerr << "\033[1;31m|" << req->uri << "|\033[0m" << std::endl;

    // custom error pages
    Directive *error_page = NULL;
    Location *location = NULL;

    std::map<std::string, Directive>::iterator eit = req->mainServ->directives.find("error_page");
    if ( eit != req->mainServ->directives.end())
        error_page = &(eit->second);

    req->uri = decode_url(req->uri);


    // remove querystring to check existance of the file
    std::string uri = req->uri;
    std::string querystr = "";
    size_t pos = req->uri.find("?");
    if (pos != std::string::npos)
    {
        querystr = req->uri.substr(pos);
        uri = req->uri.substr(0, pos);
    }
    // We should handle directories and root here
    req->uri = _GET_DELETE(*req->mainServ, uri, req->method, &location); // this give the path of the file
    std::string extension = "";

    size_t pPos = req->uri.find_last_of(".");
    if (pPos != std::string::npos)
        extension = req->uri.substr(pPos);

    // check cgi path

    if (cgiPathValid(location, extension) == 1)
        if ((!strcmp(extension.c_str(), ".php") || !strcmp(extension.c_str(), ".py") || !strncmp(extension.c_str(), ".php?", 5) || !strncmp(extension.c_str(), ".py?", 4)))
            req->IsCgi = true;

    if (req->IsCgi)
    {
        req->uri += querystr;
        M_DEBUG && std::cerr << "\033[1;31m" << req->bodyFile << "\033[0m" << std::endl;
        if (req->bodyFile.empty())
            body_fd = 0;
        else {
            body_fd = open(req->bodyFile.c_str(), O_RDONLY);
            unlink(req->bodyFile.c_str());
        }
        if (body_fd < 0)
            throw ErrorStatus(503, "Error opening body file in checkAndOpen", error_page);
        fd = CGI::responseCGI(req, body_fd, location);
    }
    else {
        if (req->method == "POST")
        {
            std::rename(req->bodyFile.c_str(), req->uri.c_str());
            unlink(req->bodyFile.c_str());
            throw SuccessStatus(201, "Uploaded file successfully", true);
        }
        else
        {
            fd = open(req->uri.c_str(), O_RDONLY);
            if (fd < 0)
                throw (ErrorStatus(500, "open failed in checkAndOpen", error_page));
        }
    }

    return fd;
}

int WebServ::handleExistedConnection(struct kevent* current)
{
    // Read and Parse Request
    HttpRequest* req = (HttpRequest*) ((t_eventData*)current->udata)->reqData;



    M_DEBUG && std::cerr << "Reading request\n";
    req->readRequest(current->ident);

    if(req->isDone == true)
    {

        // custom error pages
        Directive *error_page = NULL;

        std::map<std::string, Directive>::iterator eit = req->mainServ->directives.find("error_page");
        if ( eit != req->mainServ->directives.end())
            error_page = &(eit->second);

        
        KQueue::removeWatch(current->ident, EVFILT_READ);


        int fd = checkAndOpen(req);
        KQueue::setFdNonBlock(fd);
        
        M_DEBUG && std::cerr << "Request parsed and passed to execution\n" ;

        if (req->IsCgi) {
            t_eventData* evData = new t_eventData("cgi ready", new HttpResponse(current->ident, fd, req));
            if (KQueue::watchChildExited(req->cgiPid, evData) == -1)
                throw ErrorStatus(503, "WatchState at handleExistedConnection(1)", error_page);
            KQueue::startedCgis[evData] = std::time(NULL);
        } else {
            t_eventData* evData = new t_eventData("send response", new HttpResponse(current->ident, fd, req));
            if (KQueue::watchState(current->ident, evData, EVFILT_WRITE) == -1)
                throw ErrorStatus(503, "watchState in handleExistedConnection(2)", error_page);
            evData->resData->sendHeaders();
        }

        std::map<t_eventData*, time_t>::iterator it = KQueue::connectedClients.find((t_eventData*)current->udata);
        KQueue::connectedClients.erase(it);

        delete (t_eventData*)current->udata;
        current->udata = NULL;

        return 0;
    }

	return 0;
}

int WebServ::handleNewConnection(struct kevent* current)
{
    t_eventData *serverEvData = (t_eventData*)current->udata;
    t_sockData* tmp = (t_sockData*)serverEvData->serverData;

	int clientSock = accept(current->ident, (sockaddr*)(tmp->sockAddress), tmp->sockLen);
    if (clientSock == -1) {
        if (M_DEBUG)
            perror("accept(2)");
        return -1;
    }

    KQueue::setFdNonBlock(clientSock);

    KQueue::waitForClientToSend(clientSock, serverEvData->s);
	return 0;
}


void WebServ::cgiSwitchToSending(struct kevent* current)
{
    KQueue::removeWatch(current->ident, EVFILT_PROC);
    t_eventData* evData = (t_eventData*) current->udata;

    std::map<t_eventData*, time_t>::iterator it = KQueue::startedCgis.find(evData);
    KQueue::startedCgis.erase(it);

    int clientSocket = ((HttpResponse*)evData->resData)->clientSocket;

    int s;
    waitpid(current->ident, &s, WNOHANG);

    M_DEBUG && std::cerr << "status: " << s << '\n';

    if (s != 0)
        throw ErrorStatus(clientSocket, 500, "in cgiSwitchToSending: cgi failed");

    evData->type = "send response";

    if (KQueue::watchState(clientSocket, evData, EVFILT_WRITE) == -1)
        throw ErrorStatus(clientSocket, 503, "watchState in cgiSwitchToSending");

    evData->resData->sendHeaders();
}

void WebServ::sendResponse(struct kevent* current)
{
    t_eventData* evData = (t_eventData*) current->udata;
    HttpResponse *res = (HttpResponse*)evData->resData;

    res->sendingResponse(current->data);

    if (res->ended) {
        KQueue::removeWatch(res->clientSocket, EVFILT_WRITE);
        // Salat response kolchi daz mzyan
        
        if (res->connectionClose)
            close(res->clientSocket);
        else {
            KQueue::waitForClientToSend(res->clientSocket, res->s);
            m_watchedStates++;
        }

        m_watchedStates--;
        delete evData;
    }
}

void WebServ::loop()
{
    while (true)
    {
        struct kevent events[m_watchedStates];
        int nevents = KQueue::getEvents(events, m_watchedStates, m_watchedStates);

        for (int i = 0; i < nevents; i++) {
            try {
                if (!std::strcmp((static_cast<t_eventData*>(events[i].udata))->type, "server socket")) {

                    if (handleNewConnection(&events[i]) == -1)
                        continue ;
                    m_watchedStates++;
                    M_DEBUG && std::cerr << "Connection accepted" << std::endl ;
                }
                else if (!std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "client socket")) {
                    M_DEBUG && std::cerr << "receiving\n";
                    handleExistedConnection(&events[i]);
                }
                else if (!std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "cgi ready")) {
                    cgiSwitchToSending(&events[i]);
                }
                else if (!std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "send response")) {
                    sendResponse(&events[i]);                    
                }

            } catch(ErrorStatus& e) {
                if (!std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "client socket")
                    || !std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "send response"))
                    e.clientSock = events[i].ident;
            
                e.sendError();
                if (events[i].udata) {
                    if (std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "cgi ready")) {
                        std::map<t_eventData*, time_t>::iterator it = KQueue::connectedClients.find((t_eventData*)events[i].udata);
                        KQueue::connectedClients.erase(it);
                    }
                    delete (t_eventData*)events[i].udata;
                }
                m_watchedStates--;
            }
            catch(SuccessStatus& e) {
                if (!std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "client socket")
                    || !std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "send response"))
                    e.clientSock = events[i].ident;

                e.sendSuccess();
                if (events[i].udata) {
                    if (std::strcmp(static_cast<t_eventData*>(events[i].udata)->type, "cgi ready")) {
                        std::map<t_eventData*, time_t>::iterator it = KQueue::connectedClients.find((t_eventData*)events[i].udata);
                        KQueue::connectedClients.erase(it);
                    }
                    if (!e.connClose) {
                        if (e.clientSock > 0) {
                            KQueue::waitForClientToSend(e.clientSock, ((t_eventData*)events[i].udata)->s);
                            m_watchedStates++;
                        }
                    }
                    delete (t_eventData*)events[i].udata;
                }
                m_watchedStates--;
                // connection will be closed automatically by getting out of the scope of the catch
            }

        }
        waitpid(-1, NULL, WNOHANG); // This is for cleaning the cgi processes that terminated
    }
}

void WebServ::run()
{
    KQueue::createKq();

    try {
        for (size_t i = 0; i < servers.size(); i++) {
            servers[i].init();

            if (KQueue::watchState(servers[i].getSocket(), &(servers[i].m_sEventData), EVFILT_READ) == -1) // sockets are closed in destructor of servers
                throw std::runtime_error("There was an error while adding server socket to kqueue");
            m_watchedStates++;
        }
    } catch(std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }

    while (true) 
    {
        try {
            loop();
        } catch(std::exception& e) {
            M_DEBUG && std::cerr << e.what() << '\n';
        } catch(std::out_of_range& e) {
            M_DEBUG && std::cerr << e.what() << '\n';
        }
    }

    
}
