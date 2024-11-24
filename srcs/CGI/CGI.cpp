#include "CGI.hpp"

std::string  generateRandomFileName(const std::string path, const std::string extension) {

    std::srand(static_cast<unsigned int>(std::time(0)));

    std::time_t now = std::time(0);
    std::tm* localTime = std::localtime(&now);

    char timeBuffer[15]; // "YYYYMMDDHHMMSS"
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d%H%M%S", localTime);

    std::ostringstream oss;
    if (!path.empty()) {
        oss << path;
        if (path[path.size() - 1] != '/') {
            oss << "/";
        }
    }

    oss << ".";

    oss << timeBuffer;
    oss << "_" << (std::rand() % 10000);

    if (!extension.empty()) {
        oss << "." << extension;
    }

    return oss.str();
}

int CGI::responseCGI(HttpRequest* req, int bodyFd, Location *location) {

    // custom error pages
    Directive *error_page = NULL;

    std::map<std::string, Directive>::iterator eit = req->mainServ->directives.find("error_page");
    if ( eit != req->mainServ->directives.end())
        error_page = &(eit->second);

    std::string fileName = generateRandomFileName("/tmp/", "webserv");
    
    int outputFile = open(fileName.c_str(), O_CREAT | O_WRONLY, 0644);
    if (outputFile == -1) {
        if (M_DEBUG)
            perror("pipe(2)");
        throw ErrorStatus(503, "open(2) failed in responseCGI", error_page);
    }

    std::string scriptName = req->uri;
    std::string cgiPath = "";
    size_t queryPos = scriptName.find('?');
    if (queryPos != std::string::npos) 
        scriptName = scriptName.substr(0, queryPos);
    if (scriptName.find(".py") != std::string::npos)
    {
        if (location && location->directives.find("py-cgi") != location->directives.end())
            cgiPath = location->directives["py-cgi"].values[0];
        else if (location)
            ErrorStatus(404, "Python Cgi path not found", error_page);
    }
    else
    {
        if (location && location->directives.find("php-cgi") != location->directives.end())
            cgiPath = location->directives["php-cgi"].values[0];
        else if (location)
            ErrorStatus(404, "php Cgi path not found", error_page);
    }

    char *argv[] = {
        const_cast<char*>(cgiPath.c_str()),
        const_cast<char*>(scriptName.c_str()),
        NULL
    };

    int pid = fork();
    if (!pid)
    {
        dup2(outputFile, 1);
        dup2(bodyFd, 0);

        close(outputFile);
        close(bodyFd);



        std::string cookie = req->getHeader("Cookie");
        if (cookie != "")
            cookie.insert(cookie.size() - 2, ";");


        setenv("SCRIPT_FILENAME", scriptName.c_str(), 1);
        setenv("PATH_INFO", scriptName.c_str(), 1);
        setenv("REDIRECT_STATUS", "200", 1);
        setenv("CONTENT_TYPE", req->getHeader("Content-Type").substr(0, req->getHeader("Content-Type").size() - 2).c_str(), 1);
        setenv("REQUEST_METHOD", req->method.c_str(), 1);
        setenv("HTTP_COOKIE", cookie.c_str(), 1);

        (req->method == "POST") && setenv("CONTENT_LENGTH", std::to_string(req->content_length).c_str(), 1);
        if (queryPos != std::string::npos)
            setenv("QUERY_STRING", req->uri.substr(queryPos + 1).c_str(), 1);
        
        size_t p = scriptName.find_last_of("/");
        if (p!=std::string::npos)
            if (chdir(scriptName.substr(0, p).c_str()) == -1 && M_DEBUG)
                perror("chdir(2)");
        
        if (execve(argv[0], const_cast<char* const*>(argv), environ) == -1) {
            if (M_DEBUG)
                perror("execve(2)");
            exit(1);
        }
    }
    else
    {
        if (pid == -1) {
            if (M_DEBUG)
                perror("fork(2)");
            close(outputFile);
            close(bodyFd);
            throw ErrorStatus(503, "fork failed in responseCGI", error_page);
        }
        req->cgiPid = pid;
        close(outputFile);
        outputFile = open(fileName.c_str(), O_RDONLY, 0644);
        unlink(fileName.c_str());
        KQueue::setFdNonBlock(outputFile);
    }

    return outputFile;
}

