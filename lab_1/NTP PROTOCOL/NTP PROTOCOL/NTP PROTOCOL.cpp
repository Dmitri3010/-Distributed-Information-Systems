
#include "stdafx.h"
#include <iostream>
#include <ctime>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Winsock2.h"
#pragma comment (lib, "ws2_32.lib")


struct NTP_packet	//пакет NTP
{
	CHAR head[4];
	DWORD32 RootDelay;
	DWORD32 RootDispersion;
	CHAR ReferenceIdentifier[4];	//идентификатор
	DWORD ReferenceTimestamp[2];	//32 бита  - секунды с 01.01.1900 00:00, 32 бита - доли секунды в 2^-32 единицах
	DWORD64 OriginateTimestamp;
	DWORD32 TransmitTimestamp[2];	//32 бита  - секунды с 01.01.1900 00:00, 32 бита - доли секунды в 2^-32 единицах
	DWORD32 KeyIdentifier;			//optional
	DWORD64 MessageDigest[2];		//optional
};
int _tmain(int argc, _TCHAR* argv[])
{
	int h = CLOCKS_PER_SEC;
	clock_t t = clock();
	int d = 613608 * 3600;
	time_t ttime;
	time(&ttime);

	WSADATA wsaData;
	SOCKET s;
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("192.168.43.201"); 
	server.sin_port = htons(2000);

	NTP_packet out_buf, in_buf;
	ZeroMemory(&out_buf, sizeof(out_buf));
	ZeroMemory(&out_buf, sizeof(out_buf));
	out_buf.head[0] = 0x1B;
	out_buf.head[1] = 0x00;
	out_buf.head[2] = 4;
	out_buf.head[3] = 0xEC;
	try
	{
		if (WSAStartup(MAKEWORD(2, 0), &wsaData))	throw WSAGetLastError();
		if ((s = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET) throw WSAGetLastError();
		int lenout = 0, lenin = 0, lensockaddr = sizeof(server);

		if ((lenout = sendto(s, (char *)&out_buf, sizeof(out_buf), NULL, (sockaddr *)&server, sizeof(server))) == SOCKET_ERROR) throw WSAGetLastError();
		if ((lenin = recvfrom(s, (char *)&in_buf, sizeof(in_buf), NULL, (sockaddr *)&server, &lensockaddr)) == SOCKET_ERROR) throw WSAGetLastError();

		in_buf.ReferenceTimestamp[0] = ntohl(in_buf.ReferenceTimestamp[0]) - d;
		in_buf.TransmitTimestamp[0] = ntohl(in_buf.ReferenceTimestamp[0]) - d;
		in_buf.TransmitTimestamp[1] = ntohl(in_buf.ReferenceTimestamp[1]);		//доли секунды in_buf.TransmitTimestamp[1]/2^-32
		int ms = (int) 1000.0 * ((double)(in_buf.TransmitTimestamp[1]) / (double)0xffffffff);		//милисекунды

		if (closesocket(s) == INVALID_SOCKET) throw WSAGetLastError();
		if (WSACleanup() == SOCKET_ERROR) throw WSAGetLastError();
	}

	catch (int e)
	{
		std::cout << "error:" << e << std::endl;
	};

	return 0;
}

