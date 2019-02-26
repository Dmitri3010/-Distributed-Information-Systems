// LocalTimeServerClient.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "Winsock2.h"
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include "Errrors.h"
#pragma comment(lib, "WS2_32.lib")
using namespace std;

struct qtim
{
	DWORD64 mTime;
};

struct rtim
{
	__int64 corrTime;
};

DWORD64 GetLocalTimeM()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int _tmain(int argc, _TCHAR* argv[])
{
	GetLocalTimeM();
	setlocale(LC_CTYPE, "Russian");
	string IP = "192.168.43.175";
	cout << "			   Time sync								 " << endl;
	cout << "Enter IP local NTP-server: ";
	cin >> IP;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		throw SetErrorMsgText("Startup: ", WSAGetLastError());
	while (true) {
		int TC = 1000;
		int maxcorr = 0, mincorr = INT_MAX;
		__int64 mcorr = 0;
		DWORD64 ClientTime = 0;
		SOCKET cS;
		cout << "================================================================================" << endl;
		cout << "Enter TC: ";
		cin >> TC;

		try
		{
			if ((cS = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
				throw SetErrorMsgText("Socket: ", WSAGetLastError());

			SOCKADDR_IN serv;
			serv.sin_family = AF_INET;
			serv.sin_port = htons(2000);
			serv.sin_addr.s_addr = inet_addr(IP.c_str());

			qtim gs;
			rtim ss;
			ss.corrTime = 0;
			gs.mTime = 0;
			//первое выравнивание
			sendto(cS, (char*)&gs, sizeof(gs), 0, (sockaddr*)&serv, sizeof(serv));
			//получаем с сервера корректировку, котроая будет равна времени ибо текущее время == 0
			int lc = sizeof(serv);
			if ((recvfrom(cS, (char*)&ss, sizeof(ss), NULL, (sockaddr*)&serv, &lc)) == SOCKET_ERROR)
				throw SetErrorMsgText("Get time: ", WSAGetLastError());
			ClientTime = ss.corrTime;
			cout << " * Init sync:" << ss.corrTime << endl << endl;
			for (int i = 0; i < 10; i++)
			{
				Sleep(TC);
				//посылаем серверу запрос со своим временем
				ClientTime = gs.mTime = (DWORD64)ClientTime + TC;
				sendto(cS, (char*)&gs, sizeof(gs), 0, (sockaddr*)&serv, sizeof(serv));
				//получаем с сервера корректировку
				int lc = sizeof(serv);
				if ((recvfrom(cS, (char*)&ss, sizeof(ss), NULL, (sockaddr*)&serv, &lc)) == SOCKET_ERROR)
					throw SetErrorMsgText("RecvFrom: ", WSAGetLastError());
				DWORD64 lockTimM = GetLocalTimeM();
				ClientTime = (ClientTime + ss.corrTime);
				mcorr = (mcorr + ss.corrTime);
				maxcorr = (maxcorr < ss.corrTime) ? ss.corrTime : maxcorr;
				mincorr = (mincorr > ss.corrTime) ? ss.corrTime : mincorr;
				cout << "Client time: " << ClientTime << " Local time: " << lockTimM << " correct: " << ss.corrTime << endl;
				if (ClientTime > lockTimM)
				{
					cout << "The local time is behind the server from " << ClientTime - lockTimM << " ms" << endl;
				}
				else
				{
					cout << "Local time is ahead of that received from the server from " << lockTimM - ClientTime << " ms" << endl;
				}
				cout << "*****" << endl;
			}
			cout << endl << "delay on the client: " << TC << endl;
			cout << "avg correct: " << mcorr / 10.0 << endl;
			cout << "max correct: " << maxcorr << endl;
			cout << "min correct: " << mincorr << endl << endl;

			if (closesocket(cS) == SOCKET_ERROR)
				throw SetErrorMsgText("Closesocket: ", WSAGetLastError());
		}
		catch (string errorMsgText)
		{
			cout << endl << errorMsgText << endl;
		}

		system("pause");
	}
	if (WSACleanup() == SOCKET_ERROR)
		throw SetErrorMsgText("Cleanup: ", WSAGetLastError());
	return 0;
}

