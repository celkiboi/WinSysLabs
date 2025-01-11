// uncomment the following to change the communication standard to ASCII
//#undef UNICODE
//#undef _UNICODE

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <tchar.h>

#pragma comment(lib, "Ws2_32.lib")

#define COMMUNICATION_BUFFER 256

INT32 _tmain(INT32 argc, LPTSTR argv[])
{
	INT32 returnValue = 0;
	if (argc != 4)
	{
		_tprintf(_T("Error. Incorrect param number. Param number must be 3.\nUsage: client [NAME] [IP] [PORT])"));
		returnValue = 1;
		goto exit;
	}

	LPTSTR IpAddress = argv[2];
	UINT16 port = _ttoi(argv[3]);
	LPTSTR name = argv[1];
	UINT32 nameLength = _tcslen(name);

	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		_tprintf(_T("Cannot start Winsock. Error: %d"), WSAGetLastError());
		returnValue = 2;
		goto exit;
	}

	SOCKET communicationSocket;
	SOCKADDR_IN socketAddress;

	communicationSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (communicationSocket == INVALID_SOCKET)
	{
		_tprintf(_T("Cannot create socket. Error: %d"), WSAGetLastError());
		returnValue = 3;
		goto exit;
	}

	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);
	if (InetPton(AF_INET, IpAddress, &socketAddress.sin_addr) != 1)
	{
		_tprintf(_T("Invalid IP adress format. Error: %d"), WSAGetLastError());
		returnValue = 4;
		goto exit;
	}

	if (connect(communicationSocket, (SOCKADDR*)&socketAddress, sizeof(socketAddress)) == SOCKET_ERROR)
	{
		_tprintf(_T("Cannot connect. Error: %d"), WSAGetLastError());
		returnValue = 5;
		goto exit;
	}

	_tprintf(_T("Communication started. Using %d byte(s) to represent a character.\n"), sizeof(TCHAR));

	TCHAR* messageBuffer[COMMUNICATION_BUFFER] = { 0 };
	_stprintf_s(messageBuffer, sizeof(messageBuffer) / sizeof(TCHAR), _T("N%s"), name);
	send(communicationSocket, messageBuffer, nameLength * sizeof(TCHAR) + sizeof(TCHAR), 0);

	TCHAR* replyBuffer[COMMUNICATION_BUFFER] = { 0 };
	UINT32 bytesRecieved = 0;

	if ((bytesRecieved = recv(communicationSocket, replyBuffer, sizeof(replyBuffer) / sizeof(TCHAR), 0)) > 0)
	{
		_tprintf(_T("Server responded with a message: %s\n"), replyBuffer);
		bytesRecieved = 0;
	}

	if ((bytesRecieved = recv(communicationSocket, replyBuffer, sizeof(replyBuffer) / sizeof(TCHAR), 0)) > 0)
	{
		_tprintf(_T("Server responded with a message: %s\n"), replyBuffer);
		send(communicationSocket, _T("OK\0"), 3 * sizeof(TCHAR), 0);
	}

exit:
	if (communicationSocket != INVALID_SOCKET)
		closesocket(communicationSocket);
	WSACleanup();
	return returnValue;
}