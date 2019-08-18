#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

using namespace std;
// Don't forget to include "ws2_32.lib" in the library list.
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <iostream>

#define TIME_PORT	27015
#define NUM_OF_PACKETS 100

void printMenu();
void sendPacket(char* sendBuff, sockaddr_in* server, SOCKET *connSocket);
void recivePacket(char* recvBuff, SOCKET* connSocket);
void dealyMeasurement(char* sendBuff, char* recvBuff, sockaddr_in* server, SOCKET* connSocket);
void RTTMeasurement(char* sendBuff, char* recvBuff, sockaddr_in* server, SOCKET* connSocket);

int main()
{
	// Initialize Winsock (Windows Sockets).

	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Client: Error at WSAStartup()\n";
	}

	// Client side:
	// Create a socket and connect to an internet address.

	SOCKET connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connSocket)
	{
		cout << "Time Client: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	// For a client to communicate on a network, it must connect to a server.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called server.
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(TIME_PORT);

	// Send and receive data.

	int bytesSent = 0;
	int bytesRecv = 0;
	char sendBuff[255];
	char recvBuff[255];

	int key = 0;
	while (key != 12)
	{
		printMenu();
		cin >> key;
		sendBuff[0] = key;
		sendBuff[1] = '\0';

		switch (key)
		{
			case 4:
			{
				dealyMeasurement(sendBuff, recvBuff, &server, &connSocket);
				break;
			}

			case 5:
			{
				RTTMeasurement(sendBuff, recvBuff, &server, &connSocket);
				break;
			}

			case 12:
			{
				break;
			}

			default:
			{
				sendPacket(sendBuff, &server, &connSocket);
				recivePacket(recvBuff, &connSocket);
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Time Client: Closing Connection.\n";
	closesocket(connSocket);
}

void printMenu()
{
	cout << "1. Get full time\n";
	cout << "2. Get time without date\n";
	cout << "3. Get time since epoch\n";
	cout << "4. Get client to server delay estimation\n";
	cout << "5. Get round trip time\n";
	cout << "6. Get time without date or seconds\n";
	cout << "7. Get year\n";
	cout << "8. Get month and day\n";
	cout << "9. Get seconds since begining of month\n";
	cout << "10. Get day of year\n";
	cout << "11. Get daylight savings\n";
	cout << "12. Exit\n";
}

void dealyMeasurement(char* sendBuff, char* recvBuff, sockaddr_in* server, SOCKET* connSocket)
{
	long int prevTime = 0, currTime = 0, diffSum = 0;

	for (int i = 0; i < NUM_OF_PACKETS; i++)
	{
		sendPacket(sendBuff, server, connSocket);
	}
	for (int i = 0; i < NUM_OF_PACKETS; i++)
	{
		int bytesRecv = 0;

		// Gets the server's answer using simple recieve (no need to hold the server's address).
		bytesRecv = recv(*connSocket, recvBuff, 255, 0);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << i << ":Time Client: Error at recv(): " << WSAGetLastError() << endl;
			closesocket(*connSocket);
			WSACleanup();
			exit(0);
		}
		recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string

		if (i != 0)
		{
			currTime = atol(recvBuff);
			diffSum += currTime - prevTime;
			prevTime = currTime;
		}
		else
		{
			prevTime = atol(recvBuff);
		}

		cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n\n";
	}
	cout << "Time Client: the delay estimation is:" << diffSum / 99 << "\n\n";
}

void RTTMeasurement(char* sendBuff, char* recvBuff, sockaddr_in* server, SOCKET* connSocket)
{
	long int prevTime = 0, currTime = 0, diffSum = 0;

	for (int i = 0; i < NUM_OF_PACKETS; i++)
	{
		//start counting
		prevTime = GetTickCount();

		sendPacket(sendBuff, server, connSocket);

		// Gets the server's answer using simple recieve (no need to hold the server's address).
		int bytesRecv = recv(*connSocket, recvBuff, 255, 0);

		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
			closesocket(*connSocket);
			WSACleanup();
			exit(0);
		}

		//stop counting
		currTime = GetTickCount();
		diffSum += currTime - prevTime;
	}

	cout << "Time Client: RTT Delay is:" << diffSum / NUM_OF_PACKETS << "\n\n";
}

void sendPacket(char *sendBuff, sockaddr_in * server, SOCKET *connSocket)
{
	int bytesSent = 0;

	bytesSent = sendto((*connSocket), sendBuff, (int)strlen(sendBuff), 0, (const sockaddr *)server, sizeof((*server)));
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
		closesocket((*connSocket));
		WSACleanup();
		exit(0);
	}
	cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << (int)sendBuff[0] << "\" message.\n";
}

void recivePacket(char *recvBuff, SOCKET *connSocket)
{
	int bytesRecv = 0;
	recvBuff[0] = '\0';
	// Gets the server's answer using simple recieve (no need to hold the server's address).
	bytesRecv = recv(*connSocket, recvBuff, 255, 0);
	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(*connSocket);
		WSACleanup();
		exit(0);
	}

	recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
	cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n\n";
}