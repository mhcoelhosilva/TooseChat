#include <iostream>
#include <string>

#include "client.h"

Client::Client(std::string inHostName)
    : m_hostName(inHostName),
      m_blowfish(m_encryptionKey)
{

}

Client::Client(std::string inHostName, int inPortNumber)
    : m_hostName(inHostName), 
      m_portNumber(inPortNumber),
      m_blowfish(m_encryptionKey)
{

}

Client::Client(std::string inHostName, int inPortNumber, std::string inEncryptionKey)
    : m_hostName(inHostName), 
      m_portNumber(inPortNumber),
      m_encryptionKey(inEncryptionKey),
      m_blowfish(inEncryptionKey)
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

        BOOL noDelay = TRUE;
        if (setsockopt(m_connectSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay)))
        {
            std::cerr << "setsockopt failed with error: " << WSAGetLastError() << std::endl;
            closesocket(m_connectSocket);
            WSACleanup();
            return false;
        }

        BOOL keepAlive = TRUE;
        if (setsockopt(m_connectSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(keepAlive)))
        {
            std::cerr << "setsockopt failed with error: " << WSAGetLastError() << std::endl;
            closesocket(m_connectSocket);
            WSACleanup();
            return false;
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
        else
        {
            // Make the socket non-blocking
            DWORD nonBlocking = 1;
            if (ioctlsocket(m_connectSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
            {
                std::cerr << "ioctlsocket failed with error: " << WSAGetLastError() << std::endl;
                closesocket(m_connectSocket);
                WSACleanup();
                return false;
            }
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

    std::cout << ">";

    return true;
}

bool Client::Update()
{
    int iResult;

    // TODO: add client usernames to message and display on receiving
    static Utils::AsyncGetline asyncGL;
    std::string inputStr = asyncGL.getLine();
    static bool recvd = false;
    if (!inputStr.empty() && recvd)
    {
        inputStr = inputStr.substr(0, m_bufLen - 1);
        if (inputStr == "exit")
            return false;

        // encrypt message
        char* newSendBuf = new char[m_bufLen];
        memset(newSendBuf, 0, m_bufLen);
        memcpy(newSendBuf, inputStr.c_str(), strlen(inputStr.c_str()));
        if (!m_encryptionKey.empty())
        {
            m_blowfish.encrypt(newSendBuf, newSendBuf, sizeof(newSendBuf));
        }
        // add to send queue
        m_sendBufs.push(newSendBuf);

        std::cout << ">";
    }

    WSAPOLLFD socketFD = {};
	socketFD.fd = m_connectSocket;
	socketFD.events = POLLRDNORM | POLLWRNORM;
	socketFD.revents = 0;

    if (WSAPoll(&socketFD,1, 1) > 0)
	{
        if (socketFD.revents & POLLERR) //If error occurred on this socket
        {
            std::cout << "POLLERR" << std::endl;
            return false;
        }

        if (socketFD.revents & POLLHUP) //If poll hangup occurred on this socket
        {
            std::cout << "POLLHUP" << std::endl;
            return false;
        }

        if (socketFD.revents & POLLNVAL) //If invalid socket
        {
            std::cout << "POLLNVAL" << std::endl;
            return false;
        }

        if (socketFD.revents & POLLRDNORM) //If normal data can be read without blocking
        {
            // Try to receive data
            memset(m_recvBuf, 0, m_bufLen);
            iResult = recv(m_connectSocket, m_recvBuf, m_bufLen, 0);
            if (iResult > 0)
            {
                // decrypt message
                std::string welcomeStr = "Welcome!";
                int isWelcome = strcmp(welcomeStr.c_str(), m_recvBuf);
                if (!m_encryptionKey.empty() && isWelcome != 0)
                {
                    m_blowfish.decrypt(m_recvBuf, m_recvBuf, sizeof(m_recvBuf));
                }

                std::cout << "Received decrypted: " << m_recvBuf << std::endl;
                std::cout << ">";
                recvd = true;
            }
            else if (iResult == 0)
            {
                std::cout << "Connection closed" << std::endl;
                closesocket(m_connectSocket);
                WSACleanup();
                return false;
            }
            
            if (iResult == SOCKET_ERROR)
            {
                std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
                closesocket(m_connectSocket);
                WSACleanup();
                return false;
            }

        }

        if (socketFD.revents & POLLWRNORM) //If normal data can be written without blocking
        {
            if (m_sendBufs.empty())
                return true;

            char* sendBuf = m_sendBufs.front();

            // Send message
            iResult = send(m_connectSocket, sendBuf, m_bufLen, 0);
            if (iResult > 0)
            {
                std::cout << "Bytes Sent: " << iResult << std::endl;
                delete[] sendBuf;
                m_sendBufs.pop();
                return true;
            }

            if (iResult == SOCKET_ERROR) 
            {
                std::cerr << "send failed: " << WSAGetLastError() << std::endl;
                closesocket(m_connectSocket);
                WSACleanup();
                return false;
            }

            
        }
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