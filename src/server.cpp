#include <iostream>
#include <string>
#include <conio.h>

#include "server.h"
#include "utils.h"

Server::Server(int inPortNumber)
    : m_portNumber(inPortNumber)
{

}

bool Server::Initialize()
{
    m_socketFileDescriptors.clear();
    m_clientSockets.clear();

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &m_wsaData);
    if (iResult != 0) 
    {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return false;
    }

    // instantiate socket object for use as server
    struct addrinfo *result = NULL,
                    *addrPtr = NULL,
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
    iResult = setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));
    if (iResult == SOCKET_ERROR) 
    {
        std::cerr << "setsockopt for SO_REUSEADDR failed with error: " << WSAGetLastError() << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    // Make the socket non-blocking
    DWORD nonBlocking = 1;
    if (ioctlsocket(m_listenSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
    {
        std::cerr << "ioctlsocket failed with error: " << WSAGetLastError() << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    BOOL noDelay = TRUE;
    if (setsockopt(m_listenSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay)))
    {
        std::cerr << "setsockopt failed with error: " << WSAGetLastError() << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
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

    // The backlog parameter is set to SOMAXCONN. This value is a special constant that instructs 
    // the Winsock provider for this socket to allow a maximum reasonable number of pending connections in the queue.
    if (listen(m_listenSocket, SOMAXCONN ) == SOCKET_ERROR ) 
    {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    WSAPOLLFD listeningSocketFD = {};
	listeningSocketFD.fd = m_listenSocket;
	listeningSocketFD.events = POLLRDNORM;
	listeningSocketFD.revents = 0;
    m_socketFileDescriptors.push_back(listeningSocketFD);

    return true;
}

void Server::CloseConnection(int inConnectionIndex, std::string&& inReason)
{
    if (inConnectionIndex >= m_clientSockets.size())
    {
        std::cerr << "Server::CloseConnection: connection index out of range." << std::endl;
        return;
    }

	SOCKET clientSocket = m_clientSockets[inConnectionIndex];
    int errorCode = WSAGetLastError();
	std::cout << "[" << inReason << "][Error code: " << errorCode;
    std::cout << "] Connection lost with client socket " << clientSocket << "." << std::endl;

	m_socketFileDescriptors.erase(m_socketFileDescriptors.begin() + (inConnectionIndex + 1));
	m_tempFileDescs.erase(m_tempFileDescs.begin() + (inConnectionIndex + 1));
	closesocket(m_clientSockets[inConnectionIndex]);
	m_clientSockets.erase(m_clientSockets.begin() + inConnectionIndex);
}

bool Server::Update()
{
    // use temp variable so as to not corrupt member vector
    // (so we don't have to reset revents every time before calling WSAPoll())
    m_tempFileDescs = m_socketFileDescriptors;

    if (WSAPoll(m_tempFileDescs.data(), m_tempFileDescs.size(), 1) > 0)
	{
        // Listening

        WSAPOLLFD& listeningSocketFD = m_tempFileDescs[0];
		if (listeningSocketFD.revents & POLLRDNORM)
        {
            SOCKET newClientSocket = accept(m_listenSocket, NULL, NULL); //&clientAddr, &clientAddrLen);
            if (newClientSocket == INVALID_SOCKET) 
            {
                std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            }
            else
            {
                m_clientSockets.push_back(newClientSocket);
                m_recvBufs.push_back(new char[m_bufLen]);

                WSAPOLLFD newConnectionFD = {};
                newConnectionFD.fd = newClientSocket;
                newConnectionFD.events = POLLRDNORM | POLLWRNORM;
	            newConnectionFD.revents = 0;
                m_socketFileDescriptors.push_back(newConnectionFD);

                // add to send queue
                char* newSendBuf = new char[m_bufLen];
                std::string welcome = "Welcome!";
                memset(newSendBuf, 0, m_bufLen);
                memcpy(newSendBuf, welcome.c_str(), strlen(welcome.c_str()));
                m_sendBufs.push(newSendBuf);
            }
        }

        // Client sending/receiving

        for (int i = m_tempFileDescs.size() - 1; i >= 1; --i)
        {
            int connectionIndex = i - 1;
			SOCKET connectionSocket = m_clientSockets[connectionIndex];

            if (m_tempFileDescs[i].revents & POLLERR) //If error occurred on this socket
			{
                CloseConnection(connectionIndex, "POLLERR");
				continue;
			}

			if (m_tempFileDescs[i].revents & POLLHUP) //If poll hangup occurred on this socket
			{
                CloseConnection(connectionIndex, "POLLHUP");
				continue;
			}

			if (m_tempFileDescs[i].revents & POLLNVAL) //If invalid socket
			{
                CloseConnection(connectionIndex, "POLLNVAL");
				continue;
			}

			if (m_tempFileDescs[i].revents & POLLRDNORM) //If normal data can be read without blocking
			{
				memset(m_recvBufs[connectionIndex], 0, m_bufLen);
                int iResult = recv(m_clientSockets[connectionIndex], m_recvBufs[connectionIndex], m_bufLen, 0);

                if (iResult > 0) 
                {
                    std::cout << "Bytes received: " << iResult << std::endl;
                    std::cout << "Received: " << m_recvBufs[connectionIndex] << std::endl;

                    // add to send queue
                    char* newSendBuf = new char[m_bufLen];
                    memcpy(newSendBuf, m_recvBufs[connectionIndex], m_bufLen);
                    m_sendBufs.push(newSendBuf);
                } 
                else if (iResult == 0)
                {
                    CloseConnection(connectionIndex, "Recv==0");
					continue;
                }

				if (iResult == SOCKET_ERROR) //If error occurred on socket
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection(connectionIndex, "Recv<0");
						continue;
					}
				}

			}

            if (m_tempFileDescs[i].revents & POLLWRNORM) //If normal data can be written without blocking
            {
                if (m_sendBufs.empty())
                    continue;

                char* sendBuf = m_sendBufs.front();

                // For now, just echo the buffer back to the sender
                // TODO: send to destination
                int iSendResult = send(m_clientSockets[connectionIndex], sendBuf, m_bufLen, 0);
                if (iSendResult > 0)
                {
                    std::cout << "Sent " << iSendResult << " bytes successfully." << std::endl;
                    delete[] sendBuf;
                    m_sendBufs.pop();
                }
                if (iSendResult == SOCKET_ERROR) 
                {
                    std::cerr << "Send failed with error code: " << WSAGetLastError() << "." << std::endl;
                    continue;
                }
            }
        }


    }

    return true;
}

bool Server::Shutdown()
{
    // clean up receive buffers
    for (auto& b : m_recvBufs)
    {
        delete[] b;
    }

    // shutdown the send half of the connection since no more data will be sent
    for (int i = 0; i < m_clientSockets.size(); ++i)
    {
        if (m_clientSockets[i] != INVALID_SOCKET)
        {
            int iResult = shutdown(m_clientSockets[i], SD_SEND);
            if (iResult == SOCKET_ERROR) 
            {
                std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
                closesocket(m_clientSockets[i]);
                WSACleanup();
                return false;
            }
        }

        // cleanup
        closesocket(m_clientSockets[i]);
        WSACleanup();
    }

    return true;
}