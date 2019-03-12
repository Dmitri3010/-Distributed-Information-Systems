// ClientU.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Winsock2.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <ctime>
#include <wchar.h>

#pragma comment(lib, "WS2_32.lib")

typedef HANDLE HDFS;

using namespace std;

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
	return msgText + GetErrorMsgText(code);
};

SOCKET cS;
SOCKADDR_IN serv;
int len;

bool EnterCA(char* FileName)
{
	char inBuf[30];
	string str("enter");
	str.append(FileName);
	int c=sendto(cS, str.c_str(), str.length(), NULL, (sockaddr*)&serv, len);
	recvfrom(cS, inBuf, sizeof(inBuf), NULL, (sockaddr*)&serv, &len);
	if (strcmp(inBuf, "ok") == 0)
	{
		return true;
	}
	return false;
}

bool LeaveCA(char* FileName)
{
	string str("leave");
	str.append(FileName);
	sendto(cS, str.c_str(), str.length(), NULL, (sockaddr*)&serv, len);
	return true;
}

HDFS OpenDFSFile(char* FileName, char* FileWay)
{
	if (EnterCA(FileName))
	{
		HANDLE ptrFile = CreateFileA(FileWay, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (ptrFile != NULL)
		{
			return ptrFile;
		}
		return 0;
	}
	return 0;
}

int ReadDFSFile(HDFS hdfs, void* buf, int bufsize)
{
	int s;
	if (ReadFile(hdfs, buf, bufsize, (LPDWORD)&s, NULL))
	{
		return s;
	}
	else return -1;
}

int WriteDFSFile(HDFS hdfs, void* buf, int bufsize)
{
	int s;
	SetFilePointer(hdfs, NULL, NULL, FILE_END);
	if (WriteFile(hdfs, buf, bufsize, (LPDWORD)&s, NULL))
	{
		FlushFileBuffers(hdfs);//�������� �����
		return s;
	}
	else return -1;
}

void CloseDFSFile(HDFS hdfs, char* FileName)
{
	CloseHandle(hdfs);
	LeaveCA(FileName);
}

int _tmain(int argc, _TCHAR* argv[])
{	setlocale(LC_CTYPE, "Russian");
	
	//string IP = "172.16.192.134";
	string IP = "127.0.0.1";	
	time_t rawtime;
	struct tm * timeinfo;


	char buffer[80];                                // ������, � ������� ����� ��������� ������� �����
	cout << "ClientU" << endl;

	try
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
			throw SetErrorMsgText("Startup: ", WSAGetLastError());
		if ((cS = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
			throw SetErrorMsgText("Socket: ", WSAGetLastError());
		serv.sin_family  = AF_INET;
		serv.sin_port = htons(2000);
		serv.sin_addr.s_addr = inet_addr(IP.c_str());
		len = sizeof(serv);

		char buf[200];


		while(true)
		{			
			time(&rawtime);                               // ������� ���� � ��������
			timeinfo = localtime(&rawtime);               // ������� ��������� �����, �������������� � ���������
			ZeroMemory(buffer, 80);
			ZeroMemory(buf, 200);
			strftime(buffer, 80, "%c", timeinfo); // ����������� ������ �������
			HDFS h,d;
			h = OpenDFSFile("111", "D:\\111.txt");
			d = OpenDFSFile("222", "D:\\222.txt");
			if (h&&d)
			{
				if(ReadDFSFile(h, buf, 10)>0)
				cout<<"111: " << buf << endl;
				WriteDFSFile(h, buffer, strlen(buffer));
				WriteDFSFile(h, "\r\n", strlen("\r\n"));
				if (ReadDFSFile(d, buf, 10)>0)
				cout << "222: " << buf << endl;
				WriteDFSFile(d, buffer, strlen(buffer));
				WriteDFSFile(d, "\r\n", strlen("\r\n"));
			}
			else break;
			CloseDFSFile(h, "111");
			CloseDFSFile(d, "222");
			Sleep(5000);
		}


		if (closesocket(cS) == SOCKET_ERROR)
			throw SetErrorMsgText("Closesocket: ", WSAGetLastError());

		if (WSACleanup() == SOCKET_ERROR)         
			throw SetErrorMsgText("Cleanup: ", WSAGetLastError());
	}
	catch (string errorMsgText)
	{
		cout << endl << errorMsgText << endl;
	}
	cout << endl;
return 0;
}