#include "Response.hpp"

std::string strToLower(std::string s)
{
    std::string res("");
    
    for (std::string::iterator i = s.begin(); i < s.end(); i++) {
        res += std::tolower(*i);
    }
    return res;
}

const std::vector<char> HttpResponse::BuildResponse() {
    std::string temp = this->Version + " " + this->ResponseCode + "\r\n" + "Content-Type: " + this->ContentType + "\r\n" + "Connection: " + this->Connection + "\r\n" + "\r\n" ;
    std::vector<char> response(temp.begin(), temp.end());
    response.insert(response.end(), Body.begin(), Body.end());
    return response;
}

std::string HttpResponse::GetVersion(){
    return this->Version;
}

std::string HttpResponse::GetResponseCode(){
    return this->ResponseCode;
}

std::string HttpResponse::GetContentType(){
    return this->ContentType;
}

std::string HttpResponse::GetConnection(){
    return this->Connection;
}

const std::vector<char>& HttpResponse::GetBody(){
    return this->Body;
}


std::string WhatContentType(std::string uri) {
    
    if ( uri.rfind(".") == std::string::npos|| uri.substr(uri.rfind(".")) == ".html" || uri == "/" || uri.substr(uri.rfind(".")) == ".txt" ||
     uri.substr(uri.rfind(".")) == ".php" || uri.substr(uri.rfind(".")) == ".py" || strncmp(uri.substr(uri.rfind(".")).c_str(),".php?",5) == 0 || strncmp(uri.substr(uri.rfind(".")).c_str(),".py?",4) == 0)
    {
        return "text/html";
    }
    else if (uri.substr(uri.rfind(".")) == ".css")
    return "text/css";
    else if (uri.substr(uri.rfind(".")) == ".js")
        return "text/javascript";
    else if (uri.substr(uri.rfind(".")) == ".html")
        return "text/html";
    else if (uri.substr(uri.rfind(".")) == ".txt")
        return "text/plain";
    else if (uri.substr(uri.rfind(".")) == ".xml")
        return "application/xml";
    else if (uri.substr(uri.rfind(".")) == ".json")
        return "application/json";
    else if (uri.substr(uri.rfind(".")) == ".csv")
        return "text/csv";
    else if (uri.substr(uri.rfind(".")) == ".pdf")
        return "application/pdf";
    else if (uri.substr(uri.rfind(".")) == ".doc")
        return "application/msword";
    else if (uri.substr(uri.rfind(".")) == ".docx")
        return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    else if (uri.substr(uri.rfind(".")) == ".xls")
        return "application/vnd.ms-excel";
    else if (uri.substr(uri.rfind(".")) == ".xlsx")
        return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    else if (uri.substr(uri.rfind(".")) == ".ppt")
        return "application/vnd.ms-powerpoint";
    else if (uri.substr(uri.rfind(".")) == ".pptx")
        return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    else if (uri.substr(uri.rfind(".")) == ".jpg")
        return "image/jpeg";
    else if (uri.substr(uri.rfind(".")) == ".jpeg")
        return "image/jpeg";
    else if (uri.substr(uri.rfind(".")) == ".png")
        return "image/png";
    else if (uri.substr(uri.rfind(".")) == ".gif")
        return "image/gif";
    else if (uri.substr(uri.rfind(".")) == ".bmp")
        return "image/bmp";
    else if (uri.substr(uri.rfind(".")) == ".ico")
        return "image/x-icon";
    else if (uri.substr(uri.rfind(".")) == ".svg")
        return "image/svg+xml";
    else if (uri.substr(uri.rfind(".")) == ".webp")
        return "image/webp";
    else if (uri.substr(uri.rfind(".")) == ".mp3")
        return "audio/mpeg";
    else if (uri.substr(uri.rfind(".")) == ".wav")
        return "audio/wav";
    else if (uri.substr(uri.rfind(".")) == ".ogg")
        return "audio/ogg";
    else if (uri.substr(uri.rfind(".")) == ".flac")
        return "audio/flac";
    else if (uri.substr(uri.rfind(".")) == ".mp4")
        return "video/mp4";
    else if (uri.substr(uri.rfind(".")) == ".avi")
        return "video/x-msvideo";
    else if (uri.substr(uri.rfind(".")) == ".mov")
        return "video/quicktime";
    else if (uri.substr(uri.rfind(".")) == ".webm")
        return "video/webm";
    else if (uri.substr(uri.rfind(".")) == ".mkv")
        return "video/x-matroska";
    else if (uri.substr(uri.rfind(".")) == ".zip")
        return "application/zip";
    else if (uri.substr(uri.rfind(".")) == ".tar")
        return "application/x-tar";
    else if (uri.substr(uri.rfind(".")) == ".gz")
        return "application/gzip";
    else if (uri.substr(uri.rfind(".")) == ".rar")
        return "application/x-rar-compressed";
    else if (uri.substr(uri.rfind(".")) == ".7z")
        return "application/x-7z-compressed";
    else if (uri.substr(uri.rfind(".")) == ".eot")
        return "application/vnd.ms-fontobject";
    else if (uri.substr(uri.rfind(".")) == ".ttf")
        return "font/ttf";
    else if (uri.substr(uri.rfind(".")) == ".otf")
        return "font/otf";
    else if (uri.substr(uri.rfind(".")) == ".woff")
        return "font/woff";
    else if (uri.substr(uri.rfind(".")) == ".woff2")
        return "font/woff2";
    else if (uri.substr(uri.rfind(".")) == ".json")
        return "application/json";
    else if (uri.substr(uri.rfind(".")) == ".apk")
        return "application/vnd.android.package-archive";
    else if (uri.substr(uri.rfind(".")) == ".exe")
        return "application/x-msdownload";
    else if (uri.substr(uri.rfind(".")) == ".bin")
        return "application/octet-stream";
    else if (uri.substr(uri.rfind(".")) == ".xpi")
        return "application/x-xpinstall";
    else if (uri.substr(uri.rfind(".")) == ".msi")
        return "application/x-msi";
    else if (uri.substr(uri.rfind(".")) == ".iso")
        return "application/x-iso9660-image";
    else
        return "application/octet-stream";

}


void HttpResponse::sendingResponse(long buffSize) {

    buffSize *= 0.80; // I wont use the whole available buffSize to decrease the load on it

    char bf[buffSize];

    (iterations == 0) && lseek(responseFd, 0, SEEK_SET);
    int r = read(responseFd, bf, buffSize);

    if (r <= 0) {
        if (r == 0) {
            int s = send(clientSocket, "0\r\n\r\n", 5, 0);
            if (s == 0 || s == -1)
                connectionClose = true;
            ended = true;
            M_DEBUG && std::cerr << "The connection is ended\n";
        }
        // When using non block mode, r may returns -1 if there is no data in the fd
        // and it will normally wait but here it'll return -1
        return;
    }

    char *buff = bf;
    if (iterations == 0) {
        std::string tmp(bf, buffSize);
        size_t p = tmp.find("\r\n\r\n");
        if (p != std::string::npos) {
            p += 4;
            tmp = tmp.substr(0, p);
            r -= p;
            buff = bf + p;
            int s = send(clientSocket, tmp.c_str(), tmp.size(), 0);
            if (s == -1 || s == 0) {
                connectionClose = true;
                ended = true;
                return ;
            }
        }
    }

    std::stringstream tmp;
    tmp << std::hex << r;

    std::string chunkLenHex = tmp.str() + "\r\n";
    std::string fullMessage = chunkLenHex + std::string(buff, r) + "\r\n";

    int s = send(clientSocket, fullMessage.c_str(), fullMessage.size(), 0);
    if (s == -1) {
        if (M_DEBUG) {
            M_DEBUG && std::cerr << "the client probably disconnected -- ";
            perror("send(2)");
        }
        connectionClose = true;
        ended = true; // Treat send failure as a connection issue, I wont throw because there is no one to receive the error code
    }
    else if (s == 0) {
        connectionClose = true;
        ended = true;
    }

    iterations++;
}

void HttpResponse::sendHeaders()
{
    std::string headers = "HTTP/1.1 200 OK\r\n"
    "Connection: keep-alive\r\n"
    "Transfer-Encoding: chunked\r\n";
    if ((reqUri.find(".php") != std::string::npos || reqUri.find(".py") != std::string::npos)) {
        if (!reqIsCgi)
            headers += "Content-Type: application/octet-stream\r\n\r\n";
    } else
        headers += "Content-Type: " + ContentType + "\r\n\r\n";
    int s = send(clientSocket, headers.c_str(), headers.size(), 0);
    if (s == 0 || s == -1) {
        KQueue::removeWatch(clientSocket, EVFILT_WRITE);
        if (s == -1)
            throw ErrorStatus(clientSocket, 500, "Send failed");
        throw ErrorStatus(clientSocket, -1, "Client closed connection before sending headers");
    }

}