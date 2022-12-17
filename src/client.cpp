#include "client.h"

#include <iostream>
#include <string>

Client::Client(std::string inHostName)
: m_hostName(inHostName)
{

}

Client::Client(std::string inHostName, int inPortNumber)
: m_hostName(inHostName), 
  m_portNumber(inPortNumber)
{

}

bool Client::Initialize()
{
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &m_wsaData);
    if (iResult != 0) 
    {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return false;
    }

    // TODO: pass in as argument
    int numRetries = 3;
    while (numRetries > 0)
    {
        struct addrinfo *result = NULL,
                        hints;

        ZeroMemory( &hints, sizeof(hints) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_V4MAPPED | AI_ALL;

        // Resolve the server address and port
        std::string portNumString = std::to_string(m_portNumber);
        if (portNumString.empty() || m_hostName.empty())
            return false;

        iResult = getaddrinfo(m_hostName.c_str(), portNumString.c_str(), &hints, &result);
        if (iResult != 0)
        {
            std::cerr << "getaddrinfo failed: " << iResult << std::endl;
            WSACleanup();
            return false;
        }

        m_connectSocket = INVALID_SOCKET;

        // Attempt to connect to the first address returned by
        // the call to getaddrinfo
        if (!result)
        {
            std::cerr << "Addrrinfo result pointer is null" << std::endl;
            return false;
        }

        // Create a SOCKET for connecting to server
        m_connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);  

        if (m_connectSocket == INVALID_SOCKET) 
        {
            std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
            freeaddrinfo(result);
            --numRetries;
            continue;
        }

        // Connect to server.
        iResult = connect(m_connectSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) 
        {
            closesocket(m_connectSocket);
            m_connectSocket = INVALID_SOCKET;
            freeaddrinfo(result);
            --numRetries;
            continue;
        }

        freeaddrinfo(result);
        
        break;
    }

    if (m_connectSocket == INVALID_SOCKET) 
    {
        std::cerr << "Unable to connect to server!" << std::endl;
        WSACleanup();
        return false;
    }

    return true;
}

bool Client::Update()
{
    int iResult;

    // TODO: make length tweakable
    // and limit message length on sending side
    int recvbuflen = 512;
    char recvbuf[512];

    // TODO: add client usernames to message and display on receiving

    std::cout << ">";
    std::string data;
    std::getline(std::cin, data);
    if (!data.empty())
    {
        if (data == "exit")
            return false;

        // Send message
        iResult = send(m_connectSocket, data.c_str(), data.length(), 0);
        if (iResult == SOCKET_ERROR) 
        {
            std::cerr << "send failed: " << WSAGetLastError() << std::endl;
            closesocket(m_connectSocket);
            WSACleanup();
            return false;
        }
        std::cout << "Bytes Sent: " << iResult << std::endl;
    }

    // Try to receive data
    iResult = recv(m_connectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0)
    {
        std::cout << "Bytes received: " << iResult << std::endl;
        std::cout << "Received: " << recvbuf << std::endl;
    }
    else if (iResult == 0)
    {
        std::cout << "Connection closed" << std::endl;
    }
    else
    {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
    }

    return true;
}

bool Client::Shutdown()
{
    // shutdown the connection for sending since no more data will be sent
    // the client can still use the ConnectSocket for receiving data
    int iResult = shutdown(m_connectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) 
    {
        std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
        closesocket(m_connectSocket);
        WSACleanup();
        return false;
    }

    return true;
}