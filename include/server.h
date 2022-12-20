#include <winsock2.h>
#include <ws2tcpip.h>

class Server
{
public:
    Server(int inPortNumber);

    bool Initialize();

    bool Update();

    bool Shutdown();

private:
    int m_portNumber = 27015;

    WSADATA m_wsaData;

    SOCKET m_listenSocket = INVALID_SOCKET;
    SOCKET m_clientSocket = INVALID_SOCKET;

    int m_bufLen = 512;
    char m_recvBuf[512];
};