// ServerU.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "string.h"
#include <locale>
#include "time.h"
#include "Winsock2.h"
#include <queue>

#pragma comment(lib, "WS2_32.lib")

using namespace std;

class Link
{
public:
	char* FileName;
	char command;
	bool UsingFile;
	SOCKADDR_IN Adr;
	HANDLE WaitEvent;
	HANDLE Thread;
}; 

#define MAX_FILE 10
Link *masthr[MAX_FILE];
SOCKET sS;
int lc;

string GetErrorMsgText (int code)
{
	string msgText;

	switch (code)
	{
		case WSAEINTR:				 msgText = "������ ������� ��������\n";						  break;
		case WSAEACCES:				 msgText = "���������� ����������\n";						  break;
		case WSAEFAULT:				 msgText = "��������� �����\n";								  break;
		case WSAEINVAL:				 msgText = "������ � ���������\n";							  break;
		case WSAEMFILE:				 msgText = "������� ����� ������ �������\n";				  break;
		case WSAEWOULDBLOCK:		 msgText = "������ �������� ����������\n";					  break;
		case WSAEINPROGRESS:		 msgText = "�������� � �������� ��������\n";				  break;
		case WSAEALREADY: 			 msgText = "�������� ��� �����������\n";					  break;
		case WSAENOTSOCK:   		 msgText = "����� ����� �����������\n";						  break;
		case WSAEDESTADDRREQ:		 msgText = "��������� ����� ������������\n";				  break;
		case WSAEMSGSIZE:  			 msgText = "��������� ������� �������\n";				      break;
		case WSAEPROTOTYPE:			 msgText = "������������ ��� ��������� ��� ������\n";		  break;
		case WSAENOPROTOOPT:		 msgText = "������ � ����� ���������\n";					  break;
		case WSAEPROTONOSUPPORT:	 msgText = "�������� �� ��������������\n";					  break;
		case WSAESOCKTNOSUPPORT:	 msgText = "��� ������ �� ��������������\n";				  break;
		case WSAEOPNOTSUPP:			 msgText = "�������� �� ��������������\n";					  break;
		case WSAEPFNOSUPPORT:		 msgText = "��� ���������� �� ��������������\n";			  break;
		case WSAEAFNOSUPPORT:		 msgText = "��� ������� �� �������������� ����������\n";	  break;
		case WSAEADDRINUSE:			 msgText = "����� ��� ������������\n";						  break;
		case WSAEADDRNOTAVAIL:		 msgText = "����������� ����� �� ����� ���� �����������\n";	  break;
		case WSAENETDOWN:			 msgText = "���� ���������\n";								  break;
		case WSAENETUNREACH:		 msgText = "���� �� ���������\n";							  break;
		case WSAENETRESET:			 msgText = "���� ��������� ����������\n";					  break;
		case WSAECONNABORTED:		 msgText = "����������� ����� �����\n";						  break;
		case WSAECONNRESET:			 msgText = "����� �������������\n";							  break;
		case WSAENOBUFS:			 msgText = "�� ������� ������ ��� �������\n";				  break;
		case WSAEISCONN:			 msgText = "����� ��� ���������\n";							  break;
		case WSAENOTCONN:			 msgText = "����� �� ���������\n";							  break;
		case WSAESHUTDOWN:			 msgText = "������ ��������� send: ����� �������� ������\n";  break;
		case WSAETIMEDOUT:			 msgText = "���������� ���������� ��������  �������\n";		  break;
		case WSAECONNREFUSED:		 msgText = "���������� ���������\n";						  break;
		case WSAEHOSTDOWN:			 msgText = "���� � ����������������� ���������\n";			  break;
		case WSAEHOSTUNREACH:		 msgText = "��� �������� ��� �����\n";						  break;
		case WSAEPROCLIM:			 msgText = "������� ����� ���������\n";						  break;
		case WSASYSNOTREADY:		 msgText = "���� �� ��������\n";							  break;
		case WSAVERNOTSUPPORTED:	 msgText = "������ ������ ����������\n";					  break;
		case WSANOTINITIALISED:		 msgText = "�� ��������� ������������� WS2_32.DLL\n";		  break;
		case WSAEDISCON:			 msgText = "����������� ����������\n";						  break;
		case WSATYPE_NOT_FOUND:		 msgText = "����� �� ������\n";								  break;
		case WSAHOST_NOT_FOUND:		 msgText = "���� �� ������\n";								  break;
		case WSATRY_AGAIN:			 msgText = "������������������ ���� �� ������\n";			  break;
		case WSANO_RECOVERY:		 msgText = "�������������� ������\n";						  break;
		case WSANO_DATA:			 msgText = "��� ������ ������������ ����\n";				  break;
		case WSA_INVALID_HANDLE:	 msgText = "��������� ���������� �������  � �������\n";		  break;
		case WSA_INVALID_PARAMETER:	 msgText = "���� ��� ����� ���������� � �������\n";			  break;
		case WSA_IO_INCOMPLETE:		 msgText = "������ �����-������ �� � ���������� ���������\n"; break;
		case WSA_IO_PENDING:		 msgText = "�������� ���������� �����\n";					  break;
		case WSA_NOT_ENOUGH_MEMORY:	 msgText = "�� ���������� ������\n";						  break;
		case WSA_OPERATION_ABORTED:	 msgText = "�������� ����������\n";							  break;
		case WSAEINVALIDPROCTABLE:	 msgText = "��������� ������\n";							  break;
		case WSAEINVALIDPROVIDER:	 msgText = "������ � ������ �������\n";						  break;
		case WSAEPROVIDERFAILEDINIT: msgText = "���������� ���������������� ������\n";			  break;
		case WSASYSCALLFAILURE:		 msgText = "��������� ���������� ���������� ������\n";		  break;
		default:					 msgText = "Error\n";										  break;
	};

	return msgText;
}

string SetErrorMsgText (string msgText, int code)
{
	return  msgText + GetErrorMsgText (code);
};




DWORD WINAPI Dispath(LPVOID lp)
{
	queue<SOCKADDR_IN> *q = new queue<SOCKADDR_IN>();
	int index = (int)lp;
	while (true)
	{
		WaitForSingleObject(masthr[index]->WaitEvent,INFINITE);
		if (masthr[index]->command == 'e'){
			if (q->empty())
			{
				sendto(sS, "ok", sizeof("ok"), NULL, (sockaddr*)&masthr[index]->Adr, lc);
			}
			q->push(masthr[index]->Adr);
		}
		else if (masthr[index]->command == 'l'){
			q->pop();
			if (!q->empty()){
				SOCKADDR_IN sc = q->front();
				sendto(sS, "ok", sizeof("ok"), NULL, (sockaddr*)&sc, lc);
			}
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{	setlocale(LC_ALL, "Russian");
	
	std::cout << "ServerU" << endl;
	try
	{
		for (int i = 0; i < MAX_FILE; i++)
		{	
			masthr[i] = NULL;
		}
		WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
				throw SetErrorMsgText("Startup: ", WSAGetLastError());
			if ((sS = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
				throw SetErrorMsgText("Socket: ", WSAGetLastError());
			SOCKADDR_IN serv;
			serv.sin_family = AF_INET;
			serv.sin_port = htons(2000);
			serv.sin_addr.s_addr = INADDR_ANY;
			
			if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
				throw SetErrorMsgText("Bind_Server: ", WSAGetLastError());
			
			while(true)
			{
				SOCKADDR_IN client;
				lc = sizeof(SOCKADDR_IN);
				char ibuf[50];
				ZeroMemory(ibuf, 50);
				if ((recvfrom(sS, ibuf, sizeof(ibuf), NULL, (sockaddr*)&client, &lc)) == SOCKET_ERROR)
					throw SetErrorMsgText("RecvFrom: ", WSAGetLastError());
				int port = client.sin_port;
				std::cout << "Client (" << inet_ntoa(client.sin_addr) << ":" << port << "): " << ibuf << endl;
				bool create = true;
				for (int i = 0; i < MAX_FILE; i++)
				{
					if (masthr[i] != NULL)
					{
						if (strcmp(ibuf + 5, masthr[i]->FileName) == 0)
						{
							create = false;
							masthr[i]->command = ibuf[0];
							masthr[i]->Adr = client;
							SetEvent(masthr[i]->WaitEvent);
							break;
						}
					}
				}
				if (create)
				{
					Link* a = new Link();
					int pos = -1;
					bool findplace = false;
					for (int i = 0; i < MAX_FILE; i++)
					{
						if (masthr[i] == 0) 
						{
							findplace = true;
							pos = i;
							masthr[i] = a;
							break;
						}
					}

					if (!findplace)
					{
						std::cout << "not create new thread" << endl;
						break;
					}
					char* filename = new char[strlen(ibuf)-5];
					strcpy(filename, ibuf + 5);

					a->FileName=filename;
					a->command = ibuf[0];
					a->Adr = client;
					a->UsingFile = false;
					a->WaitEvent = CreateEvent(NULL, false, true, NULL);
					a->Thread = CreateThread(NULL, NULL,&Dispath,(void*)pos, NULL, NULL);
					SetEvent(a->WaitEvent);
				}
			}
		
		if(closesocket(sS) == SOCKET_ERROR)
			throw SetErrorMsgText("close socket: ", WSAGetLastError());
		if(WSACleanup() == SOCKET_ERROR)
			throw SetErrorMsgText("Cleanup: ", WSAGetLastError());
	}
	catch (string errorMsgText)
	{
		std::cout << endl << errorMsgText;
	}
	
	return 0;
}

