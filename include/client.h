#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>

class Client
{
public:
    Client(std::string inHostName);
    Client(std::string inHostName, int inPortNumber);

    bool Initialize();

    bool Update();

    bool Shutdown();

private:
    std::string m_hostName;
    int m_portNumber = 27015;

    WSADATA m_wsaData;

    SOCKET m_connectSocket = INVALID_SOCKET;
};