// LocalTimeServer.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>
#include <ctime>
#include "Winsock2.h"
#include <windows.h>
#include "Errrors.h"

#pragma comment(lib, "WS2_32.lib")

using namespace std;

DWORD64 GetLocalTimeM()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

clock_t lastsync;

struct qtim
{
	DWORD64 mTime;
};

struct rtim
{
	__int64 corrTime;
};

struct NTP_packet
{
	CHAR head[4];
	DWORD32 RootDelay;
	DWORD32 RootDispersion;
	CHAR ReferenceIdentifier[4];
	DWORD32 RequestTimestamp[2];
	DWORD32 ReferenceTimestamp[2];
	DWORD32 OriginateTimestamp[2];
	DWORD32 TransmitTimestamp[2];
	DWORD32 KeyIdentifier;
	DWORD32 MessageDigest[4];
};

CRITICAL_SECTION csServerTime;

DWORD WINAPI timUpdThread(void* lpar)
{
	cout << "Time server started!" << endl << endl;
	int h = CLOCKS_PER_SEC;	// clock-тиков в сек.
	clock_t t = clock();	// тики со старта
	int d = 613608 * 3600;	// 613608 * 3600 сек между 1.1.1900:00:00 и 1.1.1970:00:00
	time_t ttime; time(&ttime); // количество сек с 1.1.1970:00:00
	WSAData wsaData;
	SOCKET s;
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("178.124.134.106"); 
	server.sin_port = htons(123);

	NTP_packet out_buf, in_buf;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) throw WSAGetLastError();

	//в цикле каждые 10 секунд запрашиваем у сервера точное время
	while (true)
	{
		ZeroMemory(&out_buf, sizeof(out_buf));
		ZeroMemory(&in_buf, sizeof(in_buf));
		out_buf.head[0] = 0x1B;
		out_buf.head[1] = 0x00;
		out_buf.head[2] = 4;
		out_buf.head[3] = 0xEC;
		DWORD64 tms = 0, oms = 0;
		try {
			if ((s = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET) throw WSAGetLastError();

			DWORD timeout = 20000;
			setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(DWORD));

			int lenout = 0, lenin = 0, lensockaddr = sizeof(server);
			clock_t qtime = clock();
			if ((lenout = sendto(s, (char*)&out_buf, sizeof(out_buf), NULL, (sockaddr*)&server, sizeof(server))) == SOCKET_ERROR)
				throw  SetErrorMsgText("Send: ", WSAGetLastError());
			if ((lenin = recvfrom(s, (char*)&in_buf, sizeof(in_buf), NULL, (sockaddr*)&server, &lensockaddr)) == SOCKET_ERROR)
				throw SetErrorMsgText("Recieve: ", WSAGetLastError());
			clock_t ptime = clock() - qtime;	//время, прошедшее с момента отправки запроса до приёма ответа
			DWORD64 lockTimM = GetLocalTimeM();
			//конвертируем
			in_buf.TransmitTimestamp[0] = ntohl(in_buf.TransmitTimestamp[0]) - d;
			in_buf.OriginateTimestamp[0] = ntohl(in_buf.OriginateTimestamp[0]) - d;
			in_buf.TransmitTimestamp[1] = ntohl(in_buf.TransmitTimestamp[1]);	// доли секунды 
			in_buf.OriginateTimestamp[1] = ntohl(in_buf.OriginateTimestamp[1]);
			//считаем время обработки запроса
			tms = (DWORD64)((1000.0*((double)(in_buf.TransmitTimestamp[1]) / (double)0xffffffff)) + (DWORD64)in_buf.TransmitTimestamp[0] * 1000);	// ВРЕМЯ В МИЛИСЕКУНДАХ
			oms = (DWORD64)((1000.0*((double)(in_buf.OriginateTimestamp[1]) / (double)0xffffffff)) + (DWORD64)in_buf.OriginateTimestamp[0] * 1000);	// ВРЕМЯ В МЛМСКЕУНДАХ

			//подсчитаем корректировку на передачу
			DWORD64 corr = (ptime - ((DWORD64)tms - (DWORD64)oms)) / 2;
			//скорректируем на время передачи по сети
			tms = (DWORD64)tms + corr;

			qtim* t = (qtim*)lpar;
			EnterCriticalSection(&csServerTime);
			lastsync = clock();
			t->mTime = (DWORD64)tms;
			LeaveCriticalSection(&csServerTime);

			cout << "Sync with  NTP server" << endl;

			if (tms > lockTimM)
			{
				cout << "The local time is behind the server from " << tms - lockTimM << " ms" << endl;
			}
			else
			{
				cout << "Local time is ahead of that received from the server from " << lockTimM - tms << " ms" << endl;
			}
			cout << "*****" << endl;

			if (closesocket(s) == INVALID_SOCKET) throw WSAGetLastError();

			Sleep(14000);
		}
		catch (int e) {
			std::cout << "error:" << e << std::endl;
		}
		catch (string errorMsgText)
		{
			cout << errorMsgText;
		}

	}
	if (WSACleanup() == SOCKET_ERROR) throw WSAGetLastError();
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	InitializeCriticalSection(&csServerTime);
	qtim ExTime;
	//запускаем поток, синхронизирующий время с сервером в интернете
	HANDLE updHnd = CreateThread(0, 0, timUpdThread, &ExTime, 0, 0);
	//запускаем цикл обработки клиентских запросов
	try
	{
		SOCKET sS;
		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			throw SetErrorMsgText("Startup: ", WSAGetLastError());

		if ((sS = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
			throw SetErrorMsgText("Socket: ", WSAGetLastError());

		SOCKADDR_IN serv;
		serv.sin_family = AF_INET;
		serv.sin_port = htons(2000);
		serv.sin_addr.s_addr = INADDR_ANY;

		if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
			throw SetErrorMsgText("Bind_Server: ", WSAGetLastError());

		while (true)
		{
			SOCKADDR_IN client;
			int lc = sizeof(client);
			//получаем запрос с клиента
			qtim gs;
			if ((recvfrom(sS, (char*)&gs, sizeof(gs), NULL, (sockaddr*)&client, &lc)) == SOCKET_ERROR)
				throw SetErrorMsgText("RecvFrom: ", WSAGetLastError());
			//отправляем ответ клиенту
			rtim ss;
			//смещение относительно последней синхронизации
			EnterCriticalSection(&csServerTime);
			int delt = clock() - lastsync;
			//отправляем сразу корректировку - ВРЕМЯ С СЕРВЕРА + ВРЕМЯ, КОТОРОЕ ПРОШЛО С МОМЕНТА СИНХРОНИЗАЦИИ-время клиента
			ss.corrTime = ExTime.mTime + delt - gs.mTime;
			LeaveCriticalSection(&csServerTime);

			sendto(sS, (char*)&ss, sizeof(ss), 0, (sockaddr*)&client, sizeof(client));
		}

		if (closesocket(sS) == SOCKET_ERROR)
			throw SetErrorMsgText("close socket: ", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			throw SetErrorMsgText("Cleanup: ", WSAGetLastError());
	}
	catch (string errorMsgText)
	{
		cout << endl << errorMsgText;
	}

	WaitForSingleObject(updHnd, INFINITE);
	DeleteCriticalSection(&csServerTime);
	return 0;
}

