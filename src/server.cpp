#include <iostream>
#include <string>

#include "server.h"

#include <iostream>

Server::Server(int inPortNumber)
    : m_portNumber(inPortNumber)
{

}

bool Server::Initialize()
{
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &m_wsaData);
    if (iResult != 0) 
    {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return false;
    }

    // instantiate socket object for use as server
    struct addrinfo *result = NULL,
                    hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    // AI_PASSIVE indicates the caller intends to use the returned 
    // socket address structure in a call to the bind function (server only)
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    std::string portNumString = std::to_string(m_portNumber);
    if (portNumString.empty())
        return false;

    iResult = getaddrinfo(NULL, portNumString.c_str(), &hints, &result);
    if (iResult != 0)
    {
        std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        return false;
    }

    if (!result)
    {
        std::cerr << "Result addrinfo pointer is null!" << std::endl;
        return false;
    }

    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (m_listenSocket == INVALID_SOCKET) 
    {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    BOOL bOptVal = TRUE;
    int bOptLen = sizeof (BOOL);
    iResult = setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, bOptLen);
    if (iResult == SOCKET_ERROR) 
    {
        std::cerr << "setsockopt for SO_REUSEADDR failed with error: " << WSAGetLastError() << std::endl;
    }
    else
    {
        std::cout << "Set SO_REUSEADDR: ON" << std::endl;
    }

    // Setup the TCP listening socket
    iResult = bind(m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) 
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    // addr info no longer needed, free result
    freeaddrinfo(result);

    return true;
}

bool Server::Update()
{
    // After the socket is bound to an IP address and port on the system, 
    // the server must then listen on that IP address and port for incoming connection requests.

    // The backlog parameter is set to SOMAXCONN. This value is a special constant that instructs 
    // the Winsock provider for this socket to allow a maximum reasonable number of pending connections in the queue.
    if (listen(m_listenSocket, SOMAXCONN ) == SOCKET_ERROR ) 
    {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    /* 
     * Normally a server application would be designed to listen for connections from multiple clients. 
     * For high-performance servers, multiple threads are commonly used to handle the multiple client connections.
     * 
     * There are several different programming techniques using Winsock that can be used to listen for multiple client connections. One programming technique is to create a continuous loop that checks for connection requests using the listen function (see Listening on a Socket). If a connection request occurs, the application calls the accept, AcceptEx, or WSAAccept function and passes the work to another thread to handle the request. Several other programming techniques are possible.

     * For now, just accept one connection per loop.
     */
    if (m_clientSocket == INVALID_SOCKET)
    {
        // TODO: this needs to be done async!
        m_clientSocket = accept(m_listenSocket, NULL, NULL);
        if (m_clientSocket == INVALID_SOCKET) 
        {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            return true; // no cleanup, keep trying (?)
        }
    }

    int iResult, iSendResult;

    // Receive until the peer shuts down the connection
    memset(m_recvBuf, 0, m_bufLen);
    iResult = recv(m_clientSocket, m_recvBuf, m_bufLen, 0);
    if (iResult > 0) 
    {
        std::cout << "Bytes received: " << iResult << std::endl;
        std::cout << "Received: " << m_recvBuf << std::endl;

        // For now, just echo the buffer back to the sender
        // TODO: send to destination
        iSendResult = send(m_clientSocket, m_recvBuf, m_bufLen, 0);
        if (iSendResult == SOCKET_ERROR) 
        {
            std::cerr << "send failed: " << WSAGetLastError() << std::endl;
            closesocket(m_clientSocket);
            WSACleanup();
            return false;
        }

        std::cout << "Bytes Sent: " << iResult << std::endl;
    } 
    else if (iResult == 0)
    {
        std::cout << "Connection closed" << std::endl;
    }
    else 
    {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
        closesocket(m_clientSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool Server::Shutdown()
{
    // shutdown the send half of the connection since no more data will be sent
    if (m_clientSocket != INVALID_SOCKET)
    {
        int iResult = shutdown(m_clientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) 
        {
            std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
            closesocket(m_clientSocket);
            WSACleanup();
            return false;
        }
    }

    // cleanup
    closesocket(m_clientSocket);
    WSACleanup();

    return true;
}