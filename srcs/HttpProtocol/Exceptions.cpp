#include "Exceptions.hpp"

void ErrorStatus::setErrorMessage()
{
    if (errorpages)
    {
        std::vector<std::string>::iterator itt = find(errorpages->values.begin(), errorpages->values.end(), std::to_string(errorCode));
        if (itt !=  errorpages->values.end())
        {
            itt++;
            if (access((*itt).c_str(), F_OK | R_OK) != 0)
                throw ErrorStatus(403, "Error page Not exist or has not permission");
            std::ifstream ofs(*itt);
            if (ofs.fail())
                throw ErrorStatus(500, "Can't open Error page");
            std::string line, file = "";
            while (std::getline(ofs, line))
                file += line;
            statusMessage = headers + file;
            ofs.close();
            return ;
        }
    }

    switch (errorCode)
    {
        case -1:
            break;
        case 400:
            statusMessage = headers + "<html><head><title>400 Bad Request</title></head><body><center><h1>400 Bad Request</h1></center><hr></body></html>";
            break;
    
        case 401:
            statusMessage = headers + "<html><head><title>401 Unauthorized</title></head><body><center><h1>401 Unauthorized</h1></center><hr></body></html>";
            break;

        case 403:
            statusMessage = headers + "<html><head><title>403 Forbidden</title></head><body><center><h1>403 Forbidden</h1></center><hr></body></html>";
            break;

        case 404:
            statusMessage = headers + "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr></body></html>";
            break;

        case 405:
            statusMessage = headers + "<html><head><title>405 Method Not Allowed</title></head><body><center><h1>405 Method Not Allowed</h1></center><hr></body></html>";
            break;
        
        case 408:
            statusMessage = headers + "<html><head><title>408 Request Timeout</title></head><body><center><h1>408 Request Timeout</h1></center><hr></body></html>";
            break;

        case 409:
            statusMessage = headers + "<html><head><title>409 Conflict</title></head><body><center><h1>409 Conflict</h1></center><hr><p>The request could not be processed due to a conflict with the current state of the resource.</p></body></html>";
            break;

        case 413:
            statusMessage = headers + "<html><head><title>413 Request Entity Too Large</title></head><body><center><h1>413 Request Entity Too Large</h1></center><hr></body></html>";
            break;

        case 500:
            statusMessage = headers + "<html><head><title>500 Internal Server Error</title></head><body><center><h1>500 Internal Server Error</h1></center><hr></body></html>";
            break;

        case 501:
            statusMessage = headers + "<html><head><title>501 Not Implemented</title></head><body><center><h1>501 Not Implemented</h1></center><hr></body></html>";
            break;

        case 502:
            statusMessage = headers + "<html><head><title>502 Bad Gateway</title></head><body><center><h1>502 Bad Gateway</h1></center><hr></body></html>";
            break;

        case 503:
            statusMessage = headers + "<html><head><title>503 Service Unavailable</title></head><body><center><h1>503 Service Unavailable</h1></center><hr></body></html>";
            break;

        case 504:
            statusMessage = headers + "<html><head><title>504 Gateway Timeout</title></head><body><center><h1>504 Gateway Timeout</h1></center><hr></body></html>";
            break;
        // Add more status codes
        // ..
        default:
            M_DEBUG && std::cerr << "Error: Unknown success code\n";
            statusMessage = headers + "<html><head><title>500 Internal Error</title></head><body><center><h1>500 Internal Error</h1></center><hr></body></html>";
            break;
    }
}

ErrorStatus::ErrorStatus(int clienSocket, int errorCode, const char* debugMsg)
    : clientSock (clienSocket)
    , errorCode (errorCode)
    , headers ("HTTP/1.1 " + std::to_string(errorCode) + "\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n")
    , errorpages (NULL)
{
    if (M_DEBUG && debugMsg)
        std::cerr << debugMsg << "\n";
    setErrorMessage();
}

ErrorStatus::ErrorStatus(int errorCode, const char* debugMsg)
    : clientSock (-1)
    , errorCode (errorCode)
    , headers ("HTTP/1.1 " + std::to_string(errorCode) + "\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n")
    , errorpages (NULL)
{
    if (M_DEBUG && debugMsg)
        std::cerr << debugMsg << "\n";
    setErrorMessage();
}

ErrorStatus::ErrorStatus(int errorCode, const char* debugMsg, Directive *errorpages)
    : clientSock (-1)
    , errorCode (errorCode)
    , headers ("HTTP/1.1 " + std::to_string(errorCode) + "\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n")
    , errorpages (errorpages)
{
    if (M_DEBUG && debugMsg)
        std::cerr << debugMsg << "\n";
    setErrorMessage();
}

// Success
void SuccessStatus::setSuccessMessage()
{
    switch (successCode)
    {
        case -1:
            break;
        case 200:
            statusMessage = headers + "<html><head><title>200 OK</title></head><body><center><h1>200 OK</h1></center><hr></body></html>";
            break;
        
        case 201:
            statusMessage = headers + "<html><head><title>201 Created</title></head><body><center><h1>201 Created</h1></center><hr></body></html>";
            break;

        case 202:
            statusMessage = headers + "<html><head><title>202 Accepted</title></head><body><center><h1>202 Accepted</h1></center><hr></body></html>";
            break;
    
        case 301:
            statusMessage = headers + "<html><head><title>301 Moved Permanently</title></head><body><center><h1>301 Moved Permanently</h1></center><hr><p>The requested resource has been permanently moved to <a href='" + retur + "'>" + retur + "</a>.</p></body></html>";
            break;

        case 302:
            statusMessage = headers + "<html><head><title>302 Found</title></head><body><center><h1>302 Found</h1></center><hr><p>The requested resource has been temporarily moved to <a href='" + retur + "'>" + retur + "</a>.</p></body></html>";
            break;
        
        // Add more status codes
        // ..
        default:
            M_DEBUG && std::cerr << "Error: Unknown success code\n";
            throw ErrorStatus(clientSock, 500, NULL);
            break;
    }
}

SuccessStatus::SuccessStatus(int successCode, const char* debugMsg, bool connClose)
    : clientSock (-1)
    , successCode (successCode)
    , headers ("HTTP/1.1 " + std::to_string(successCode) + "\r\nContent-Type: text/html\r\nConnection: " + (!connClose ? "keep-alive" : "close") +"\r\n\r\n")
    , retur("")
    , connClose(connClose)
{
    if (M_DEBUG && debugMsg)
        std::cerr << debugMsg << "\n";
    setSuccessMessage();
}

SuccessStatus::SuccessStatus(int successCode, const char* debugMsg, std::string retur, bool connClose)
    : clientSock (-1)
    , successCode (successCode)
    , headers ("HTTP/1.1 " + std::to_string(successCode) + "\r\nContent-Type: text/html\r\nLocation: " + retur + "\r\nConnection: " + (!connClose ? "keep-alive" : "close") +"\r\n\r\n")
    , retur(retur)
    , connClose(connClose)
{
    if (M_DEBUG && debugMsg)
        std::cerr << debugMsg << "\n";
    setSuccessMessage();
}

SuccessStatus::SuccessStatus(int clienSocket, int successCode, const char* debugMsg, bool connClose)
    : clientSock (clienSocket)
    , successCode (successCode)
    , headers ("HTTP/1.1 " + std::to_string(successCode) + "\r\nContent-Type: text/html\r\nConnection: " + (!connClose ? "keep-alive" : "close") + "\r\n\r\n")
    , retur("")
    , connClose(connClose)
{
    if (M_DEBUG && debugMsg)
        std::cerr << debugMsg << "\n";
    setSuccessMessage();
}