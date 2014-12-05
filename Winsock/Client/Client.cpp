#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

// http://msdn.microsoft.com/en-us/library/windows/desktop/bb530750(v=vs.85).aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms737591(v=vs.85).aspx

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// __cdecl is the default calling convention for C and C++ programs.
// Because the stack is cleaned up by the caller, it can do vararg functions. 
// The __cdecl calling convention creates larger executables than __stdcall, 
// because it requires each function call to include stack cleanup code.
// This specifier is Microsoft-specific and we should not use them if we want to write portable code.
// http://www.codeproject.com/Articles/1388/Calling-Conventions-Demystified

int __cdecl main(int argc, char **argv) 
{
	// All processes (applications or DLLs) that call Winsock functions must initialize 
	// the use of the Windows Sockets DLL before making other Winsock functions calls. 
	// This also makes certain that Winsock is supported on the system.
    WSADATA wsaData;

	// Declare a SOCKET object called ConnectSocket for connecting to server.
    SOCKET ConnectSocket = INVALID_SOCKET;

	// Declare two pointers to and an object of addrinfo type that contains a sockaddr structure. 
	// The information specified in the sockaddr structure includes:
	// - The IP address of the server that the client will try to connect to.
	// - The port number on the server that the client will connect to. 
	//   This port was specified as port 27015 when the client called the getaddrinfo function.

	int iResult;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    char *sendbuf = "Hello from Winsock client. This is a test message.\0";
    char recvbuf[DEFAULT_BUFLEN];
    int const recvbuflen = DEFAULT_BUFLEN;
    
	// Validate the parameters
	// if (argc != 2) 
	// {
	//	   printf("usage: %s server-name\n", argv[0]);
	//     return 1;
	// }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) 
	{
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }


	// --- Creating a Socket for the Client ---

	// Initialize sockaddr structure. 
	// For this application, the Internet address family is unspecified so that either an IPv6 or IPv4 address can be returned. 
	// The application requests the socket type to be a stream socket for the TCP protocol.
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    // --- Resolving the Server Address and Port ---

	// Call the getaddrinfo function requesting the IP address for the server name passed on the command line. 
	// The TCP port on the server that the client will connect to is defined by DEFAULT_PORT as 27015 in this sample. 
	// The getaddrinfo function returns its value as an integer that is checked for errors.

	// The getaddrinfo function is used to determine the values in the sockaddr structure. 
	// In this example, the first IP address returned by the getaddrinfo function is used to specify 
	// the sockaddr structure passed to the connect. If the connect call fails to the first IP address, 
	// then try the next addrinfo structure in the linked list returned from the getaddrinfo function.

	// iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
    
    if ( iResult != 0 ) 
	{
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

	// Attempt to connect to the first address returned by the call to getaddrinfo until one succeeds.
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET object for connecting to server.
		// Call the socket function and return its value to the ConnectSocket variable. 
		// For this application, use the first IP address returned by the call to getaddrinfo function 
		// that matched the address family, socket type, and protocol specified in the hints parameter. 
		// In this example, a TCP stream socket was specified with a socket type of SOCK_STREAM and 
		// a protocol of IPPROTO_TCP. The address family was left unspecified (AF_UNSPEC), so the 
		// returned IP address could either be an IPv6 or IPv4 address for the server.
		// If the client application wants to connect using only IPv6 or IPv4, then the address family 
		// needs to be set to AF_INET6 for IPv6 or AF_INET for IPv4 in the hints parameter.
		// The parameters passed to the socket function can be changed for different implementations.
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        
		// Check for errors to ensure that the socket is a valid socket.
		// Error detection is a key part of successful networking code. 
		// If the socket call fails, it returns INVALID_SOCKET. 
		// The if statement is used to catch any errors that may have occurred while creating the socket. 
		// WSAGetLastError returns an error number associated with the last error that occurred.
		if (ConnectSocket == INVALID_SOCKET) 
		{
            printf("Socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }


		// --- Connecting to a Server Socket ---

		// For a client to communicate on a network, it must connect to a server.
		// Call the connect function, passing the created socket and the sockaddr structure as parameters. 
		// Try the next address returned by getaddrinfo if the connect call failed.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		// Check for general errors.
        if (iResult == SOCKET_ERROR)
		{
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

		// Should really try the next address returned by getaddrinfo if the connect call failed.
		// But for this example we just free the resources returned by getaddrinfo and print an error message.
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) 
	{
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }


	// --- Sending and Receiving Data on the Client ---

    // Send an initial buffer.
	// The send and recv functions both return an integer value of the number of bytes sent 
	// or received, respectively, or an error. Each function also takes the same parameters: 
	// the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.

    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );

    if (iResult == SOCKET_ERROR) 
	{
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

	// --- Disconnecting the Client ---
    
	// When the client is done sending data to the server, the shutdown function 
	// can be called specifying SD_SEND to shutdown the sending side of the socket. 
	// This allows the server to release some of the resources for this socket. 
	// The client application can still receive data on the socket.
	// Shutdown the connection since no more data will be sent.
	// Shutdown the send half of the connection since no more data will be sent.
    iResult = shutdown(ConnectSocket, SD_SEND);

    if (iResult == SOCKET_ERROR) 
	{
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive data until the server closes the connection.
    do {

		// BLock until recv function returns
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        if ( iResult > 0 )
            printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

	// Cleanup
	// When the client application is done receiving data, 
	// the closesocket function is called to close the socket.
    closesocket(ConnectSocket);

	// WSACleanup is used to terminate the use of the WS2_32 DLL.
	// When the client application is completed using the Windows Sockets DLL, 
	// the WSACleanup function is called to release resources.
    WSACleanup();

	cin.get();

    return 0;
}