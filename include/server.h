#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <queue>

class Server
{
public:
    Server(int inPortNumber);
    Server(int inPortNumber, int inMaxNumClients);

    bool Initialize();

    bool MultiThreadUpdate(int inIdx);
    bool Update();

    bool Shutdown();

    void CloseConnection(int inConnectionIndex, std::string&& inReason);

private:
    int m_portNumber = 27015;

    WSADATA m_wsaData;

    SOCKET m_listenSocket = INVALID_SOCKET;
    std::vector<SOCKET> m_clientSockets;
    std::vector<WSAPOLLFD> m_socketFileDescriptors;
    std::vector<WSAPOLLFD> m_tempFileDescs;
    int m_maxNumClients = 16;

    int m_bufLen = 512;
    std::vector<char*> m_recvBufs;
    // TODO: change to struct that includes sender and destination data and any other metadata
    std::queue<char*> m_sendBufs;
};