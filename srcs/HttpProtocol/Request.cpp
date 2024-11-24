#include "Request.hpp"
#include "../server/Server.hpp"

static std::string ft_strtrim(std::string& str)
{
    unsigned long x;
    unsigned long y;
    y = 0;
    x = 0;
    for (; x < str.size(); x++)
        if (str[x] != ' ' && str[x] != '\r' && str[x] != '\t')
            break;
   for (y = str.size() - 1; y >= 0; y--)
        if (str[y] != ' ' && str[y] != '\r' && str[y] != '\t')
            break;
    return str.substr(x, y - x + 1);
}

void HttpRequest::generateUniqueFile(void)
{
    static int i = 0;
    if (i == INT_MAX)
        i = 0;
    // std::string file_name = "temp_" + std::to_string(i) + ".txt";
    std::string file_name = generateRandomFileName("", "");
    std::ofstream file(file_name);
    if (file.is_open())
    {
        file.close();
        bodyFile = file_name;
        i++;
    }
    else
        throw ErrorStatus(500, "generateUniqueFile()");
}

void HttpRequest::parseHeaders(std::string line)
{
    std::string key;
    std::string value;

    key     = line.substr(0, line.find(": "));
    value   = line.substr(line.find(": ") + 1, line.size() - 1);

    key = ft_strtrim(key);
    value = ft_strtrim(value);

    if (key == "Content-Length")
        content_length = std::atoi(value.c_str());
    else if (key == "Transfer-Encoding")
        TransferEncoding = value;
    else
        headers[key] = value;
}

void HttpRequest::parseFirstLine(std::string line)
{
    std::vector<std::string> tokens;
    std::stringstream tokensStream(line);
    std::string token;

    while (std::getline(tokensStream, token, ' '))
        tokens.push_back(token);

    if (tokens.size() != 3)
        throw ErrorStatus(400, "parseFirstLine()");

    method = tokens[0];
    uri = tokens[1];
    version = tokens[2];
}

HttpRequest::HttpRequest()
    : isDone(false)
    , content_length(-1)
    , total_read_bytes(0)
    , chunk_size(0)
    , bodyRead(0)
    , state(FirstLine)
    , read_bytes(0)
    , chunkPos(0)
    , TransferEncoding("")
    , skipNextLine(false)
    , IsCgi(false)
    , mainServ(NULL)
{
        partial_data.reserve(1);
        cgiPid = -1;
}

void HttpRequest::parseRequest(const std::string& request)
{
    std::string line(request);

    if (line == "\r\n" && state == Headers)
    {

        // Get main Server Here

        // looking for the right server
        std::string host = this->getHeader("Host");

        if (host.back() == '\n')
            host.pop_back();
        if (host.back() == '\r')
            host.pop_back();

        if ((*this->s).size() == 1)
        {
            this->mainServ = &(*this->s)[0];
        }
        else
        {
            for (size_t i = 0; i < this->s->size(); i++)
            {
                for (size_t t = 0; t < (*this->s)[i].directives["server_name"].values.size(); t++)
                {
                    if ((*this->s)[i].directives["server_name"].values[t] == host)
                    {
                        this->mainServ = &(*this->s)[i];
                        break ;
                    }
                }
            }
        }

        if (this->mainServ == NULL)
            this->mainServ = &(*this->s)[0];



        if (method == "POST")
        {
            // custom error pages
            Directive *error_page = NULL;

            std::map<std::string, Directive>::iterator eit = this->mainServ->directives.find("error_page");
            if ( eit != this->mainServ->directives.end())
                error_page = &(eit->second);
            if (mainServ->directives.find("client_max_body_size") != mainServ->directives.end() && this->content_length > std::atoi(mainServ->directives["client_max_body_size"].values[0].c_str()))
                throw ErrorStatus(413, "Passed max body size", error_page);
            state = Body;
            generateUniqueFile();
            
            // If it's a POST with no content-length and no chunked encoding, it's malformed
            if (content_length == -1 && TransferEncoding.empty())
                throw ErrorStatus(400, "Missing Content-Length or Transfer-Encoding");
        }
        else
        {
            // If it's not a POST request, we're done after headers
            isDone = true;
        }
        return;
    }

    switch (state)
    {
        case FirstLine:
            parseFirstLine(line);
            state = Headers;
            break;
        case Headers:
            parseHeaders(line);
            break;
        case Body:
            if (TransferEncoding == "chunked\r\n")
                unchunkBody(const_cast<char*>(request.c_str()), request.size());
            else
                parseBody(const_cast<char*>(request.c_str()), request.size());
    }
}
void HttpRequest::unchunkBody(char *request, size_t size)
{
    while (size > 0)
    {
        if (chunk_size == 0)
        {
            try {
                // Find the end of the chunk size line
                char *endOfSize = strstr(request, "\r\n");
                if (endOfSize == NULL) {
                    M_DEBUG && std::cerr << "\033[1;31mIncomplete chunk size line\033[0m" << std::endl;
                    return;
                }

                // Parse the chunk size
                *endOfSize = '\0'; // Temporarily null-terminate the chunk size string
                chunk_size = std::strtol(request, NULL, 16);
                *endOfSize = '\r'; // Restore the original string

                M_DEBUG && std::cerr << "\033[1;32mChunk size: " << chunk_size << "\033[0m" << std::endl;

                // Move the request pointer past the chunk size line
                size_t sizeLineLength = endOfSize - request + 2; // +2 for \r\n
                request += sizeLineLength;
                size -= sizeLineLength;

                // If we receive a chunk of size 0, it means we're done
                if (chunk_size == 0) {
                    isDone = true;
                    return;
                }
            }
            catch(const std::exception& e){
                M_DEBUG && std::cerr << "\033[1;31mFailed to parse chunk size: " << e.what() << "\033[0m" << std::endl;
                return;
            }
        }

        // Calculate the number of bytes to write
        size_t bytesToWrite = std::min<size_t>(size, chunk_size - chunkPos);
        parseBody(request, bytesToWrite);

        // Update positions
        chunkPos += bytesToWrite;
        request += bytesToWrite;
        size -= bytesToWrite;

        // If we've written the entire chunk, reset for the next chunk
        if (chunkPos == chunk_size)
        {
            chunk_size = 0;
            chunkPos = 0;

            // Move past the trailing \r\n of the chunk
            if (size >= 2 && request[0] == '\r' && request[1] == '\n') {
                request += 2;
                size -= 2;
            } else {
                M_DEBUG && std::cerr << "\033[1;31mMissing chunk terminator\033[0m" << std::endl;
                return;
            }
        }
    }
}

void HttpRequest::parseBody(char *line, size_t size)
{
    // For regular POST requests, make sure we don't exceed content_length
    if (TransferEncoding.empty() && content_length != -1)
    {
        size_t remaining = content_length - total_read_bytes;
        size = std::min(size, remaining);
    }

    total_read_bytes += size;
    
    std::ofstream file(bodyFile, std::ios::app | std::ios::binary);
    if (file.is_open())
    {
        file.write(line, size);
        file.close();  // Make sure to close the file after writing
    }
    else
        throw ErrorStatus(500, "parseBody()");
}

void HttpRequest::readRequest(int fd) {
    const size_t buffer_size = 1024;
    char buffer[buffer_size] = {};
    std::vector<char> crlf;
    crlf.push_back('\r');
    crlf.push_back('\n');

    read_bytes = recv(fd, buffer, buffer_size, 0);
    if (read_bytes <= 0)
    {
        if (read_bytes == 0 && state == FirstLine)
            throw SuccessStatus(-1, "", true);
        if (read_bytes == 0)
            isDone = true;
        if (read_bytes == -1)
            throw ErrorStatus(clientSocket, 500, "recv(2) failed in readRequest");

        return;
    }


    // Add new data to our partial buffer
    partial_data.insert(partial_data.end(), buffer, buffer + read_bytes);

    // If we're already processing the body for a non-chunked request
    if (state == Body && TransferEncoding.empty())
    {
        parseBody(partial_data.data(), partial_data.size());
        partial_data.clear();
        
        // Check if we've received all the data
        if (total_read_bytes >= content_length)
        {
            isDone = true;
        }
        return;
    }

    // Process headers line by line
    if (state != Body)
    {
        std::vector<char>::iterator pos = std::search(partial_data.begin(), partial_data.end(), crlf.begin(), crlf.end());
        while (pos != partial_data.end() && state != Body)
        {
            std::vector<char> line(partial_data.begin(), pos + 2);
            std::string strLine(line.begin(), line.end());
            parseRequest(strLine);
            partial_data.erase(partial_data.begin(), pos + 2);

            // If we've just transitioned to Body state, break the line processing
            if (state == Body)
                break;

            pos = std::search(partial_data.begin(), partial_data.end(), crlf.begin(), crlf.end());
        }
    }


    // If we're in Body state and have remaining data
    if (state == Body && !partial_data.empty())
    {
        if (TransferEncoding == "chunked\r\n")
        {
            // Process chunked data
            std::vector<char>::iterator pos = std::search(partial_data.begin(), partial_data.end(), crlf.begin(), crlf.end());
            while (pos != partial_data.end())
            {
                std::vector<char> line(partial_data.begin(), pos + 2);
                std::string strLine(line.begin(), line.end());
                parseRequest(strLine);
                partial_data.erase(partial_data.begin(), pos + 2);

                if (isDone)
                    break;

                pos = std::search(partial_data.begin(), partial_data.end(), crlf.begin(), crlf.end());
            }
        }
        else
        {
            // For regular POST, process all remaining data as body
            parseBody(partial_data.data(), partial_data.size());
            partial_data.clear();
            if (total_read_bytes >= content_length)
            {
                isDone = true;
            }
        }
    }
}
