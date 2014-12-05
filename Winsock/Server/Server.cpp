#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

// http://msdn.microsoft.com/en-us/library/windows/desktop/bb530751(v=vs.85).aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(void) 
{
	// All processes (applications or DLLs) that call Winsock functions must initialize 
	// the use of the Windows Sockets DLL before making other Winsock functions calls. 
	// This also makes certain that Winsock is supported on the system.
    WSADATA wsaData;

	// Declare a SOCKET object called ListenSocket for the server to listen for client connections.
    SOCKET ListenSocket = INVALID_SOCKET;

	// Declare a temporary SOCKET object called ClientSocket for accepting connections from clients.
    SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo hints;
    struct addrinfo *result = NULL;
    
	int iResult;
    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    const int recvbuflen = DEFAULT_BUFLEN;
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) 
	{
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

	// --- Creating a Socket for the Server ---

	// The getaddrinfo function is used to determine the values in the sockaddr structure:
	// - AF_INET is used to specify the IPv4 address family.
	// - SOCK_STREAM is used to specify a stream socket.
	// - IPPROTO_TCP is used to specify the TCP protocol .
	// - AI_PASSIVE flag indicates the caller intends to use the returned socket address structure 
	//   in a call to the bind function. When the AI_PASSIVE flag is set and nodename parameter to 
	//   the getaddrinfo function is a NULL pointer, the IP address portion of the socket address 
	//   structure is set to INADDR_ANY for IPv4 addresses or IN6ADDR_ANY_INIT for IPv6 addresses.
	// - 27015 is the port number associated with the server that the client will connect to.

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

    if ( iResult != 0 ) 
	{
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

	// Call the socket function and return its value to the ListenSocket variable. 
	// For this server application, use the first IP address returned by the call to getaddrinfo 
	// that matched the address family, socket type, and protocol specified in the hints parameter. 
	// In this example, a TCP stream socket for IPv4 was requested with an address family of IPv4, 
	// a socket type of SOCK_STREAM and a protocol of IPPROTO_TCP. 
	// So an IPv4 address is requested for the ListenSocket.
	// If the server application wants to listen on IPv6, then the address family needs to be set 
	// to AF_INET6 in the hints parameter. If a server wants to listen on both IPv6 and IPv4, two 
	// listen sockets must be created, one for IPv6 and one for IPv4. These two sockets must be handled 
	// separately by the application. 
	// Windows Vista and later offer the ability to create a single IPv6 socket that is put in dual stack
	// mode to listen on both IPv6 and IPv4. For more information on this feature, see Dual-Stack Sockets.

	// Create a SOCKET object called ListenSocket for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// Check for errors to ensure that the socket is a valid socket.
    if (ListenSocket == INVALID_SOCKET) 
	{
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

	// --- Binding a Socket ---

	// For a server to accept client connections, it must be bound to a network address within the system. 
	// The following code demonstrates how to bind a socket that has already been created to an IP address and port. 
	// Client applications use the IP address and port to connect to the host network.

	// The sockaddr structure holds information regarding the address family, IP address, and port number.
	// Call the bind function, passing the created socket and sockaddr structure returned from the getaddrinfo 
	// function as parameters.

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	// Check for general errors.
    if (iResult == SOCKET_ERROR) 
	{
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

	// Once the bind function is called, the address information returned by the getaddrinfo function is no longer needed. 
	// The freeaddrinfo function is called to free the memory allocated by the getaddrinfo function for this address information.
    freeaddrinfo(result);


	// --- Listening on a Socket ---

	// After the socket is bound to an IP address and port on the system, the server 
	// must then listen on that IP address and port for incoming connection requests.

	// Call the listen function, passing as parameters the created socket and a value 
	// for the backlog, maximum length of the queue of pending connections to accept. 
	// In this example, the backlog parameter was set to SOMAXCONN. 
	// This value is a special constant that instructs the Winsock provider for this 
	// socket to allow a maximum reasonable number of pending connections in the queue. 
    iResult = listen(ListenSocket, SOMAXCONN);

	// Check the return value for general errors.
    if (iResult == SOCKET_ERROR) 
	{
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }


	// --- Accepting a Connection ---

	// Once the socket is listening for a connection, the program must handle connection requests on that socket.

	// Normally a server application would be designed to listen for connections from multiple clients. 
	// For high-performance servers, multiple threads are commonly used to handle the multiple client connections.
	// There are several different programming techniques using Winsock that can be used to listen for multiple 
	// client connections. One programming technique is to create a continuous loop that checks for connection 
	// requests using the listen function (see Listening on a Socket). If a connection request occurs, the 
	// application calls the accept, AcceptEx, or WSAAccept function and passes the work to another thread to 
	// handle the request. Several other programming techniques are possible.
	// Note that this basic example is very simple and does not use multiple threads. 
	// The example also just listens for and accepts only a single connection.

	// When the client connection has been accepted, a server application would normally pass the accepted client 
	// socket (the ClientSocket variable in the above sample code) to a worker thread or an I/O completion port and 
	// continue accepting additional connections. In this basic example, the server continues to the next step.

	// There are a number of other programming techniques that can be used to listen for and accept multiple connections. 
	// These include using the select or WSAPoll functions. Examples of some of these various programming techniques are 
	// illustrated in the Advanced Winsock Samples included with the Microsoft Windows Software Development Kit (SDK).

	// Note: On Unix systems, a common programming technique for servers was for an application to listen for connections. 
	// When a connection was accepted, the parent process would call the fork function to create a new child process to 
	// handle the client connection, inheriting the socket from the parent. 
	// This programming technique is not supported on Windows, since the fork function is not supported. 
	// This technique is also not usually suitable for high-performance servers, since the resources needed 
	// to create a new process are much greater than those needed for a thread.

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);

    if (ClientSocket == INVALID_SOCKET) 
	{
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	
	// No longer need server socket
    closesocket(ListenSocket);

	
	// --- Receiving and Sending Data on the Server ---

	// The following code demonstrates the recv and send functions used by the server.

	// The send and recv functions both return an integer value of the number of bytes sent 
	// or received, respectively, or an error. Each function also takes the same parameters: 
	// the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.

	// Keep receiving until the peer shuts down the connection
	do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

        if (iResult > 0) 
		{
            printf("Bytes received: %d\n", iResult);

			recvbuf[iResult] = '\0';

			printf("Text received: %s\n", recvbuf);

			// Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);

            if (iSendResult == SOCKET_ERROR) 
			{
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }

            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
		{
            printf("Connection closing...\n");
		}
        else  
		{
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);


	// --- Disconnecting the Server ---

	// Once the server is completed receiving data from the client and sending data back 
	// to the client, the server disconnects from the client and shutdowns the socket.

	// When the server is done sending data to the client, the shutdown function 
	// can be called specifying SD_SEND to shutdown the sending side of the socket. 
	// This allows the client to release some of the resources for this socket. 
	// The server application can still receive data on the socket.

    // Shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);

    if (iResult == SOCKET_ERROR) 
	{
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

	// Cleanup
	// When the client application is done receiving data, the closesocket function is called to close the socket.
    closesocket(ClientSocket);

	// When the client application is completed using the Windows Sockets DLL, 
	// the WSACleanup function is called to release resources.
    WSACleanup();

	cin.get();

    return 0;
}