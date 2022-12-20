#include <iostream>
#include <string>
#include <getopt.h>

#include "client.h"
#include "server.h"

void commandLineHelp(std::string inProgramName, std::ostream& inOutputStream)
{
    inOutputStream << "Usage: " << std::endl;
    inOutputStream << inProgramName << " [options]" << std::endl;
    inOutputStream << std::endl;
    inOutputStream << "Options: " << std::endl;
    inOutputStream << " --client <host-name>        = Run as client." << std::endl;
    inOutputStream << " --server                    = Run as server." << std::endl;
    inOutputStream << " --port <port-number>        = Port number to use." << std::endl;
    inOutputStream << " --key <encryption-key>      = Encryption key to use." << std::endl;
    inOutputStream << " --help                      = See usage instructions." << std::endl;
}

int main(int argc, char* argv[])
{
    bool runAsClient = false;
    std::string hostName = "";
    int portNumber = 27015;
    std::string key = "aabb09182736ccdd";

    const char* const shortOptions = "c:sp::k::h";
    static struct option longOptions[] = {
        {"client",  required_argument, 0, 0},
        {"server",  no_argument,       0, 0},
        {"port",    optional_argument, 0, 0},
        {"key",     optional_argument, 0, 0},
        {"help",    no_argument,       0, 0}
    };

    while (true)
    {
        const auto opt = getopt_long(argc, argv, shortOptions, longOptions, nullptr);

        if (-1 == opt)
            break;

        switch (opt)
        {
        case 'c':
            runAsClient = true;
            hostName = std::string(optarg);
            std::cout << "Running as client" << std::endl;
            break;

        case 's':
            runAsClient = false;
            std::cout << "Running as server" << std::endl;
            break;

        case 'p':
            portNumber = std::stoi(optarg);
            std::cout << "Port set to: " << portNumber << std::endl;
            break;

        case 'k':
            key = std::string(optarg);
            std::cout << "Using encryption key: " << key << std::endl;
            break;

        case 'h':
            commandLineHelp(argv[0], std::cout);
            break;

        case '?': // Unrecognized option
            std::cerr << "Unrecognized option" << std::endl;
            commandLineHelp(argv[0], std::cerr);
            break;

        default:
            commandLineHelp(argv[0], std::cerr);
            break;
        }
    }

    if (runAsClient)
    {
        Client c(hostName, portNumber, key);
        c.Initialize();

        while (c.Update())
        {
            Sleep(10);
        }

        c.Shutdown();
    }
    else
    {
        Server s(portNumber);

        s.Initialize();

        while (s.Update())
        {
            Sleep(10);
        }

        s.Shutdown();
    }
    
    return 0;
}