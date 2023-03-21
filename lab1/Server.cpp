#include <iostream>
#include <winsock.h>
#include <string>
#include <cstring>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")

#define MaxSize 5
SOCKET ClientSocket[MaxSize];
int index = 0;//全局变量，当前连接数

using namespace std;

struct Buffer {
	char name[16];
	char time[32];
	char content[1024];
};

string getTime() {
	time_t timep;
	time(&timep);
	char tmp[32];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
	return tmp;
}

DWORD WINAPI handler_Recv(LPVOID lparam) {
	SOCKET recvSocket = (SOCKET)(LPVOID)lparam;
	Buffer recvBuf;
	while (1) {
		if (recvSocket == INVALID_SOCKET) {
			closesocket(recvSocket);
			break;
		}
		memset((char*)&recvBuf, 0, sizeof(recvBuf));
		recv(recvSocket, (char*)&recvBuf, sizeof(recvBuf), 0);
		if (strcmp(recvBuf.content, "depart") == 0) {
			cout << recvBuf.time << " " << recvBuf.name << "离开了聊天室" << endl;
			for (int i = 0; i < index; i++) {
				if (ClientSocket[i] != INVALID_SOCKET && ClientSocket[i] != recvSocket)
					send(ClientSocket[i], (char*)&recvBuf, sizeof(recvBuf), 0);
			}
			closesocket(recvSocket);
			break;
		}
		else {
			cout << recvBuf.time << " " << recvBuf.name << ": " << recvBuf.content << endl;
			for (int i = 0; i < index; i++) {
				if (ClientSocket[i] != INVALID_SOCKET)
					send(ClientSocket[i], (char*)&recvBuf, sizeof(recvBuf), 0);
			}
		}
		
	}
	return 0;
}


int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSADATA初始化失败，错误码 : %d", WSAGetLastError());
		return 1;
	}
	printf("WSAStartup Complete!\n");

	char IPAddress[16];
	cout  << "Local Machine IP Address:";
	cin >> IPAddress;
	USHORT uPort;
	cout  << "Input the port for wait the connections from the Clients:";
	cin >> uPort;


	SOCKADDR_IN ServerAddr;
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(uPort);
	ServerAddr.sin_addr.S_un.S_addr = inet_addr(IPAddress);


	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET) {
		printf("Socket创建失败，错误码 : %d", WSAGetLastError());
	}
	cout << "Socket created!\n";

	int n = bind(ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (n == SOCKET_ERROR)
	{
		cout << "failed to bind!" << endl;
		return -1;
	}
	cout  << "Bind Success!" << endl;

	SOCKADDR_IN ClientAddr;
	int ClientAddrlen = sizeof(ClientAddr);
	cout << "Start listening..." << endl;
	int l = listen(ServerSocket, MaxSize);
	if (l != 0) {
		cout << "error" << GetLastError() << l << endl;
	}

	cout << "Starting accepting..." << endl;
	while (1) {
		if (index < MaxSize) {
			ClientSocket[index] = accept(ServerSocket, (SOCKADDR*)&ClientAddr, &ClientAddrlen);
			HANDLE hThread_recv = CreateThread(NULL, NULL, &handler_Recv, LPVOID(ClientSocket[index]), 0, NULL);
			index++;
			CloseHandle(hThread_recv);
		}
		else {
			cout << "当前人数已满" << endl;
		}
	}
	closesocket(ServerSocket);
	WSACleanup();
	return 0;
}
