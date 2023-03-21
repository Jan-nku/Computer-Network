#ifndef _SERVER_H
#define _SERVER_H

#include <iostream>
#include <fstream>
#include <winsock.h>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define Port 6666
#define MAX_WAIT_TIME 5000
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

#pragma once(1)
struct Head {
	u_char type;//状态位
	u_char seq;//序列号
	u_short checksum;//校验和
	u_short length;//数据长度

	//接收窗口、紧急窗口
	//optional字段 
};
#pragma once()
extern WSADATA wsaData;
extern SOCKET Server;
extern SOCKADDR_IN ServerAddr, ClientAddr;
extern clock_t beg;
extern string FileName;
extern u_char Seq;
extern char FileBuf[200000000];
extern int FileSize;
extern char sendBuf[2000000];
extern char recvBuf[2000000];
const int lossRatio = 0.01;
void init();
void release();
int random();
bool ShakeHand();
bool WaveHand();
u_short CheckSum(u_short* buf, int length);
string RecvFileName();
void recvData();
BOOL lossInLossRatio(float lossRatio); 
#endif // !_SERVER_H
