

#include "configFile.hpp"

bool    isAllowedDirectiveServ(std::string direc)
{
    std::string arr[9] = {"listen", "server_name", "host", "error_page", "client_max_body_size", "root", "index", "autoindex", "return"};

    for (int i = 0; i < 9; i++)
    {
        if (arr[i] == direc)
            return (1);
    }
    return (0);
}


bool    isAllowedDirectiveloc(std::string direc)
{
    std::string arr[8] = {"autoindex", "allow_methods", "return", "php-cgi", "root", "index", "py-cgi", "upload_store"};

    for (int i = 0; i < 8; i++)
    {
        if (arr[i] == direc)
            return (1);
    }
    return (0);
}


bool    isEmpty(std::string str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (!std::isspace(str[i]))
            return (0);
    }
    return (1);
}

std::string trimStr(std::string str)
{
    std::string newStr = "";

    int start = 0;
    while ((size_t)start < str.size() && std::isspace(str[start]))
        start++;
    
    int end = str.size() - 1;
    while (end >= 0 && std::isspace(str[end]))
        end--;
    return (str.substr(start,  end - start + 1));
}

bool    invalidBrackets(std::vector<std::string>& conf)
{
    std::stack<char> par;

    for (size_t i = 0; i < conf.size(); i++)
    {
        for (size_t j = 0; j < conf[i].size(); j++)
        {
            if (conf[i][j] == '{')
                par.push(conf[i][j]);
            if (conf[i][j] == '}')
            {
                if (par.empty())
                    return (1);
                par.pop();
            }
        }
    }

    if (!par.empty())
        return (1);

    return (0);
}

std::vector<std::string>    split(std::string str, char delem)
{
    std::vector<std::string> v;
    std::string part;


    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == delem)
        {
            if (!part.empty())
            {
                v.push_back(part);
                part.clear();
            }
        }
        else
            part += str[i];
    }

    if (!part.empty())
        v.push_back(part);
    return (v);
}

int checkDirValue(std::vector<std::string> &directive)
{
    int nb;
    std::string str;

    for (size_t i = 1; i < directive.size(); i++)
    {
        str = directive[i];
        if (i == directive.size() - 1)
        {
            if (str.back() == ';')
                str.pop_back();
        }

        if (directive[0] == "listen" || directive[0] == "client_max_body_size" || directive[0] == "error_page")
        {
            std::stringstream ss(str);
            ss >> nb;
            if ((directive[0] != "error_page" || (directive[0] == "error_page" && 3 != directive.size())) && (ss.fail() || !ss.eof()))
                return (-1);
            if (directive[0] == "client_max_body_size" && directive.size() != 2)
                return (-1);
            if (directive[0] == "listen" && directive.size() != 2)
                return (-1);
            if (directive[0] == "listen" && (nb < 1024 || nb > 65535))
                return (-1);
        }
        else if (directive[0] == "autoindex")
        {
            if ((str != "on" && str != "off") || directive.size() != 2)
                return (-1);
        }
        else if (directive[0] == "allow_methods")
        {
            if (str != "GET" && str != "POST" && str != "DELETE")
                return (-1);
        }
        else if ((directive[0] == "php-cgi" || directive[0] == "py-cgi"))
        {
            if (directive.size() != 2)
                return (-1);
        }
        else if (directive[0] == "upload_store")
        {
            if (directive.size() != 2)
                return (-1);
        }
        else if (directive[0] == "return")
        {
            if (directive.size() != 3 || (directive[1] != "301" && directive[1] != "302"))
                return (-1);
        }
    }
    return (1);
}

int checkLocation(std::vector<std::string> &conf, size_t i, Location &loc)
{
    std::vector<std::string> directive;

    for ( ; i < conf.size(); i++)
    {

        if (isEmpty(conf[i]))
            continue ;

        directive = split(conf[i], ' ');
        if (directive[directive.size() - 1] == ";" && directive.size() - 2 >= 0 && directive[directive.size() - 2].back() != ';')
        {
            directive[directive.size() - 2].push_back(';');
            directive.pop_back();
        }

        if (checkDirValue(directive) == -1)
        {
            M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid directive" << std::endl;
            return (-1);
        }

        if (directive.size() == 1 && directive[0] == "}")
        {
            return (i);
        }
        else
        {
            if (directive[directive.size() - 1][directive[directive.size() - 1].size() - 1] != ';')
            {
                M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": missing semi-colon" << std::endl;
                return (-1);
            }
            else
            {
                if (isAllowedDirectiveloc(directive[0]))
                {
                    for (size_t j = 1; j < directive.size(); j++)
                    {
                        if (j == directive.size() - 1 && directive[j].back() == ';')
                            directive[j].pop_back();
                        if (directive[j].empty())
                        {
                            M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid directive" << std::endl;
                            return (-1);
                        }
                        if ( directive[0] != "error_page" &&  loc.directives.find(directive[0]) != loc.directives.end() && j == 1)
                        {
                            M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": duplacate directive" << std::endl;
                            return (-1);
                        }
                        loc.directives[directive[0]].values.push_back(directive[j]);
                    }
                }
                else
                {
                    M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid directive" << std::endl;
                    return (-1);
                }
            }
        }
    }

    return (i);
}


int checkDirectives(std::vector<std::string> &conf, size_t i, VirtualServer &serv)
{

    std::vector<std::string> directive;

    for ( ; i < conf.size(); i++)
    {

        if (isEmpty(conf[i]))
            continue ;

        directive = split(conf[i], ' ');

        if (directive[directive.size() - 1] == ";" && directive.size() - 2 >= 0 && directive[directive.size() - 2].back() != ';')
        {
            directive[directive.size() - 2].push_back(';');
            directive.pop_back();
        }

        if (checkDirValue(directive) == -1)
        {
            M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid directive" << std::endl;
            return (-1);
        }

        if (directive.size() > 0 && directive[0] == "location")
        {
            if (directive.size() != 3 || directive[0] != "location" || directive[2] != "{")
            {
                M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid location block" << std::endl;
                return (-1);
            }
            Location loc;
            int n = checkLocation(conf, i + 1, loc);
            if (n == -1)
                return (-1);
            serv.locations[directive[1]] = loc;
            i = n;
        }
        else if (directive.size() == 1 && directive[0] == "}")
        {
            return (i);
        }
        else
        {
            if (directive[directive.size() - 1][directive[directive.size() - 1].size() - 1] != ';')
            {
                M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": missing semi-colon" << std::endl;
                return (-1);
            }
            else
            {
                if (isAllowedDirectiveServ(directive[0]))
                {
                    for (size_t j = 1; j < directive.size(); j++)
                    {
                        if (j == directive.size() - 1 && directive[j].back() == ';')
                            directive[j].pop_back();
                        if (directive[j].empty())
                        {
                            M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid directive" << std::endl;
                            return (-1);
                        }
                        if ( directive[0] != "error_page" &&  serv.directives.find(directive[0]) != serv.directives.end() && j == 1)
                        {
                            M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": duplacate directive" << std::endl;
                            return (-1);
                        }
                        serv.directives[directive[0]].values.push_back(directive[j]);
                    }
                }
                else
                {
                    M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid directive" << std::endl;
                    return (-1);
                }
            }
        }
    }

    return (i);
}

bool    sameNameChecker(std::vector<Server> &servers, VirtualServer &serv)
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        if (servers[i].serv[0].directives["listen"].values[0] == serv.directives["listen"].values[0]
        && servers[i].serv[0].directives["host"].values[0] == serv.directives["host"].values[0])
            return (1);
    }
    return (0);
}

int checkServerBlock(std::vector<std::string> &conf, WebServ &main)
{

    std::vector<std::string> block;

    for (size_t i = 0; i < conf.size(); i++)
    {

        if (isEmpty(conf[i]))
            continue ;

        block = split(conf[i], ' ');

        if (block.size() != 2 || block[0] != "server" || block[1] != "{")
        {
            M_DEBUG && std::cerr << "Syntaxe error in line " << i + 1 << ": invalid server block" << std::endl;
            return (-1);
        }
        VirtualServer serv;
        int n = checkDirectives(conf, i + 1, serv);
        if (n == -1)
            return (-1);

        if (serv.directives.find("listen") == serv.directives.end())
        {
            M_DEBUG && std::cerr << "Syntaxe error : missing listen directive" << std::endl;
            return (-1);
        }

        if (serv.directives.find("root") == serv.directives.end())
        {
            M_DEBUG && std::cerr << "Syntaxe error : missing root directive" << std::endl;
            return (-1);
        }

        if (serv.directives.find("host") == serv.directives.end())
        {
            M_DEBUG && std::cerr << "Syntaxe error : missing host directive" << std::endl;
            return (-1);
        }

        if (serv.directives.find("server_name") != serv.directives.end())
            serv.server_name = serv.directives["server_name"].values[0];
        if (!main.servers.size() || !sameNameChecker(main.servers, serv))
        {
            Server s;
            s.serv.push_back(serv);
            main.servers.push_back(s);
        }
        else
        {
            for (size_t q = 0; q < main.servers.size(); q++)
            {
                if (main.servers[q].serv[0].directives["listen"].values[0] == serv.directives["listen"].values[0]
                 && main.servers[q].serv[0].directives["host"].values[0] == serv.directives["host"].values[0])
                {
                    main.servers[q].serv.push_back(serv);
                    break ;
                }
            }
        }
        i = n;   
    }

    return (1);
}


int parseConfigFile(std::string config, WebServ &main)
{
    std::ifstream file(config);

    if (!file)
        return (0);
    
    std::vector<std::string> confLines;
    std::string line;
    while (std::getline(file, line))
    {
        confLines.push_back(trimStr(line));
    }


    if (invalidBrackets(confLines))
    {
        std::cerr << "Syntaxe error : invalid brackets" <<std::endl;
        return (-1);
    }

    if (checkServerBlock(confLines, main) == -1)
        return (-1);

    if (main.servers.size() < 1)
    {
        std::cerr << "Syntaxe error : no server block" <<std::endl;
        return (-1);
    }

    return (1);
}