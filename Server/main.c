// uncomment the following to change the communication standard to ASCII
// doing so will allow for Unix compatability (and Windows on ASCII clients) but will make clients compiled with UNICODE incompatible
//#undef UNICODE
//#undef _UNICODE

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <tchar.h>

#include "errors.h"

#pragma comment(lib, "Ws2_32.lib")

#define CLIENT_COUNT 4
#define COMMUNICATION_BUFFER 256

UINT32 numberOfConnections = 0;
HANDLE maxClientsReachedEvent;

typedef struct ThreadParam
{
	UINT8 clientNumber;
	SOCKET clientSocket;
}THREAD_PARAMS;

DWORD WINAPI HandleClientConnection(LPVOID lpParam)
{
	THREAD_PARAMS* params = (THREAD_PARAMS*)lpParam;
	SOCKET clientSocket = params->clientSocket;
	UINT8 clientNumber = params->clientNumber;

	TCHAR buffer[COMMUNICATION_BUFFER] = { 0 };
	UINT32 bytesRecieved = 0;

	if ((bytesRecieved = recv(clientSocket, buffer, sizeof(buffer) / sizeof(TCHAR), 0)) > 0)
	{
		_tprintf(_T("Client number %d sends the following message: %s\n"), clientNumber + 1, buffer);
		bytesRecieved = 0;

		TCHAR* reply[3] = { 0 };
		_stprintf_s(reply, 3 * sizeof(TCHAR), _T("B%d"), clientNumber + 1);
		send(clientSocket, reply, 3 * sizeof(TCHAR), 0);
	}

	if ((clientNumber + 1) == CLIENT_COUNT)
	{
		SetEvent(maxClientsReachedEvent);
	}

	WaitForSingleObject(maxClientsReachedEvent, INFINITE);

	send(clientSocket, _T("F\0"), 2 * sizeof(TCHAR), 0);
	if ((bytesRecieved = recv(clientSocket, buffer, sizeof(buffer) / sizeof(TCHAR), 0)) > 0)
	{
		_tprintf(_T("Client number %d sends the following message: %s\n"), clientNumber + 1, buffer);
		bytesRecieved = 0;
	}

	return 0;
}

INT32 _tmain(INT32 argc, LPTSTR argv[])
{
	UINT32 returnValue = 0;
	if (argc != 3)
	{
		_tprintf(_T("Error. Incorrect param number. Param number must be 2.\nUsage: server [IP] [PORT]\nExample: server 127.0.0.1 8000\nIf you want to use automatic IP detection write IP adress as negative integer\nExample: server -1 8000\n(runs the first automatically detected IP, for second use -2 etc...)"));
		return INVALID_ARG_NUMBER;
	}

	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		_tprintf(_T("Cannot start Winsock. Error: %d"), WSAGetLastError());
		return CANNOT_START_WSA;
	}

	LPTSTR IpAddress = argv[1];

	UINT16 port = _ttoi(argv[2]);
	if (_ttoi(argv[1]) < 0)
	{
		char hostname[256];
		if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
		{
			_tprintf(_T("Failed to get hostname. Error: %d\n"), WSAGetLastError());
			returnValue = CANNOT_GET_HOSTNAME;
			goto exit;
		}

		struct hostent* host = gethostbyname(hostname);
		if (!host)
		{
			_tprintf(_T("Failed to get host by name. Error: %d\n"), WSAGetLastError());
			returnValue = CANNOT_GET_HOST_BY_NAME;
			goto exit;
		}

		struct in_addr* addr = (struct in_addr*)host->h_addr_list[_ttoi(argv[1]) * -1];
		TCHAR buffer[256];
		_stprintf_s(buffer, sizeof(buffer) / sizeof(TCHAR), _T("%d.%d.%d.%d\0"),
			(addr->S_un.S_un_b.s_b1),
			(addr->S_un.S_un_b.s_b2),
			(addr->S_un.S_un_b.s_b3),
			(addr->S_un.S_un_b.s_b4)
		);

		IpAddress = &buffer;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listenSocket == INVALID_SOCKET)
	{
		_tprintf(_T("Cannot create a listening socket. Error: %d"), WSAGetLastError());
		returnValue = CANNOT_CREATE_SOCKET;
		goto exit;
	}

	SOCKADDR_IN listenSocketAddr = { 0 };
	listenSocketAddr.sin_family = AF_INET;
	listenSocketAddr.sin_port = htons(port);
	if (InetPton(AF_INET, IpAddress, &listenSocketAddr.sin_addr) != 1)
	{
		_tprintf(_T("Invalid IP adress format. Error: %d"), WSAGetLastError());
		returnValue = INVALID_IP_ADDR_FORMAT;
		goto exit;
	}
	
	if (bind(listenSocket, (SOCKADDR*)&listenSocketAddr, sizeof(listenSocketAddr)) == SOCKET_ERROR)
	{
		_tprintf(_T("Listen socket bind failed. Error: %d\n"), WSAGetLastError());
		returnValue = SOCKET_BIND_FAIL;
		goto exit;
	}

	SOCKET clientSockets[CLIENT_COUNT] = { 0 };
	for (UINT32 i = 0; i < CLIENT_COUNT; i++)
	{
		clientSockets[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (clientSockets[i] == INVALID_SOCKET)
		{
			_tprintf(_T("Cannot create a client socket. Error: %d"), WSAGetLastError());
			returnValue = CANNOT_CREATE_SOCKET;
			goto exit;
		}
	}

	maxClientsReachedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (maxClientsReachedEvent == NULL)
	{
		_tprintf(_T("Cannot initialize event for max clients."));
		returnValue = CANNOT_INIT_MAX_CLIENTS_EVENT;
		goto exit;
	}

	listen(listenSocket, CLIENT_COUNT);
	_tprintf(_T("SERVER STARTED ON: %s:%d\nUsing %d byte(s) to represent a character.\n"), IpAddress, port, sizeof(TCHAR));
#ifdef UNICODE
	_tprintf(_T("WARNING: Using %d bytes to represent characters will result in errors on Unix clients\nChange the source code by uncommenting UNICODE undefinitions to allow for Unix compatability\n"), sizeof(TCHAR));
#endif

	UINT32 numberOfConnections = 0;
	HANDLE clientThreadHandles[CLIENT_COUNT] = { 0 };
	THREAD_PARAMS threadParams[CLIENT_COUNT] = { 0 };
	for (UINT32 i = 0; i < CLIENT_COUNT;)
	{
		clientSockets[i] = accept(listenSocket, NULL, NULL);
		if (clientSockets[i] == INVALID_SOCKET)
		{
			_tprintf(_T("Cannot accept a client socket. Error: %d\n"), WSAGetLastError());
			continue;
		}

		threadParams[i].clientNumber = i;
		threadParams[i].clientSocket = clientSockets[i];

		clientThreadHandles[i] = CreateThread(NULL, 0, HandleClientConnection, (LPVOID)&threadParams[i], 0, NULL);
		i++;
	}
	
	WaitForMultipleObjects(CLIENT_COUNT, &clientThreadHandles, TRUE, INFINITE);

exit:
	for (UINT32 i = 0; i < CLIENT_COUNT; i++)
	{
		if (clientSockets[i] != INVALID_SOCKET)
			closesocket(clientSockets[i]);
	}

	if (listenSocket != INVALID_SOCKET)
		closesocket(listenSocket);

	if (maxClientsReachedEvent != NULL)
		CloseHandle(maxClientsReachedEvent);

	WSACleanup();
	return returnValue;
}