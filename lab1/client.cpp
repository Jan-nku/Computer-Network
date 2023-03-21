#include <iostream>
#include <winsock.h>
#include <string>
#include <cstring>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
char name[16];

string getTime() {
    time_t timep;
    time(&timep);
    char tmp[32];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    return tmp;
}

struct Buffer {
    char name[16];
    char time[32];
    char content[1024];
};

DWORD WINAPI handler_Recv(LPVOID lparam) {
    SOCKET recvSocket = (SOCKET)(LPVOID)lparam;
    Buffer recvBuf;
    while (1) {
        memset((char*)&recvBuf, 0, sizeof(recvBuf));
        recv(recvSocket, (char*)&recvBuf, sizeof(recvBuf), 0);
        if (strcmp(recvBuf.content, "depart") == 0) {
            cout << recvBuf.time << " " << recvBuf.name << "离开了聊天室" << endl;
        }
        else {
            cout << recvBuf.time << " " << recvBuf.name << ": " << recvBuf.content << endl;
        }
    }
    closesocket(recvSocket);
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
    cout << "WSAStartup Complete!\n";

    char IPAddress[16];
    cout << "Input the IP of server:";
    cin >> IPAddress;
    USHORT uPort;
    cout << "Input the port of server:";
    cin >> uPort;

    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket == INVALID_SOCKET) {
        printf("Socket创建失败，错误码 : %d", WSAGetLastError());
    }
    cout << "Socket created!\n";

    SOCKADDR_IN ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(uPort);
    ServerAddr.sin_addr.S_un.S_addr = inet_addr(IPAddress);

    string str = "\n";
    getline(cin, str);

    cout << "请输入您的姓名:";
    cin.getline(name, 16);
    cout << "等待连接..." << endl;
    if (connect(ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) < 0) {
        cout << "连接失败" << endl;
        return -1;
    }
    cout << "连接成功，欢迎您用户" << name << endl;
    
    Buffer tmp;
    memset((char*)&tmp, 0, sizeof(tmp));
    strcpy(tmp.name, name);
    string time = getTime();
    strcpy(tmp.time, time.c_str());
    strcpy(tmp.content, "enters the chat room");
    send(ClientSocket, (char*)&tmp, sizeof(tmp), 0);
  
    HANDLE hThread_recv = CreateThread(NULL, NULL, &handler_Recv, LPVOID(ClientSocket), 0, NULL);
    CloseHandle(hThread_recv);
    
    Buffer sendBuf;
    while (1) {
        memset((char*)&sendBuf, 0, sizeof(sendBuf));
        strcpy(sendBuf.name, name);
        cin.getline(sendBuf.content, sizeof(sendBuf.content));
        string time = getTime();
        strcpy(sendBuf.time, time.c_str());
        send(ClientSocket, (char*)&sendBuf, sizeof(sendBuf), 0);
        if (strcmp(sendBuf.content, "depart") == 0) {
            cout << "You have disconnect with the server" << endl;
            break;
        }
    }
    closesocket(ClientSocket);
    return 0;
}
