#include "includes/webserv.h"

int main(int ac, char *av[])
{
    signal(SIGPIPE, SIG_IGN);

    int confErr;
    WebServ    main;
    std::string conf;

    if (ac == 1)
        conf = "./configs/default_conf.conf";
    else
        conf = static_cast<std::string>(av[1]);
    confErr = parseConfigFile(conf, main);
    if (confErr == 0)
    {
        std::cerr << "Invalid config file" <<std::endl;
        return (EXIT_FAILURE);
    }

    if (confErr == -1)
        return (EXIT_FAILURE);


    main.run();
    




    return (0);
}