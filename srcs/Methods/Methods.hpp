#pragma once

#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include "../main/WebServ.hpp"
#include "../server/Server.hpp"
#include <algorithm>
#include <ctime>


std::string    _GET_DELETE(VirtualServer &serv, std::string requestPath, std::string _Method, Location **location);
// std::string sessionIdGen(Server &Serv);
std::string getSessionIdFromRequest(std::string cookies);
bool    cgiPathValid(Location *location, std::string extension);
std::string _POST(std::map<std::string, Location>::iterator &it, std::string &requestPath, Directive *error_page);