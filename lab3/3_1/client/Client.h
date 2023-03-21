#ifndef _CLIENT_H
#define _CLIENT_H
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <fstream>
#include <winsock.h>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
using namespace std;

//Macro definition
#define Port 6666
#define IP "127.0.0.1"
#define MAX_WAIT_TIME 2000
#define MAX_SEND_SIZE 10000
#define FILE 0x20
#define SEND 0x10
#define LAST 0x08
#define SYN 0x04
#define FIN 0x02
#define ACK 0x01

#define SHAKE_HAND1 SYN
#define SHAKE_HAND2 SYN|ACK
#define SHAKE_HAND3 ACK
#define WAVE_HAND1 FIN|ACK
#define WAVE_HAND2 ACK
#define WAVE_HAND3 FIN|ACK
#define WAVE_HAND4 ACK
#define SEND_LAST SEND|LAST

//Global variable declaration
#pragma once(1)
struct Head {
	u_char type;//状态位
	u_char seq;//序列号,目前0-255
	u_short checksum;//校验和
	u_short length;//数据长度
	//接收窗口、紧急窗口
	//optional字段
};
#pragma once()
extern WSAData wsaData;
extern SOCKET Client;
extern SOCKADDR_IN ServerAddr;
extern u_char Seq;
extern clock_t Begin;
extern clock_t SendTime;

extern string FileName;
extern char FileBuf[200000000];
extern int FileSize;
extern char sendBuf[2000000];
extern char recvBuf[2000000];
extern int SendNum;
extern int TotalNum;


//Function declaration
void init();
void release();
bool ShakeHand();
bool WaveHand();
u_short CheckSum(u_short* buffer, int length);
void SendFileName(string SendFileName);
void Send_Message(char* data, int length);
void ReadFile();


#endif // !_CLIENT_H
