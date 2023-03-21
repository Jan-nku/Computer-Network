#pragma once

//库文件
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <fstream>
#include <winsock.h>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include<queue>
using namespace std;

//宏定义
#define Port 6665					//服务器(Router)端口
#define IP "127.0.0.1"				//服务器(Router)IP

#define MAX_WAIT_TIME 100			//最大等待时间100ms
#define MAX_SEND_SIZE 10000			//单次最大发送数据量10000Byte
#define WINDOW 10					//接收通告窗口大小，固定值10

//状态位
#define OVER 0x10
#define SEND 0x08
#define SYN 0x04
#define FIN 0x02
#define ACK 0x01

//声明全局变量
//Socket通讯
extern WSAData wsaData;
extern SOCKET Client;
extern SOCKADDR_IN ServerAddr;

//序列号
extern u_char Seq;

//文件读写
extern string FileName;				//文件名
extern u_int FileSize;				//文件大小
extern char Buffer[100000000];		//文件缓冲区
extern char sendBuf[20000];			//发送数据缓冲区

//GBN需要使用的全局变量
extern int resendCount;				// 重传次数
extern int base;					// base之前序列号累计确认
extern u_char nextSeq;				// 要发送的下一个序列号
extern bool resend;					// 是否超时重传
extern bool restart;				// 是否重新开始计时
extern bool wait;					// 是否因窗口不够而需等待
extern long long lenCopy;			// 文件数据偏移量的拷贝，供重传时使用

//拥塞控制
extern const int rwnd;				// 接收通告窗口大小，固定值
extern double cwnd;					// 拥塞窗口大小
extern int ssthresh;				// 阈值
extern unsigned char lastAck;		// 上一个ACK序列号
extern int dupAck;					// 重复收到的ACK次数
extern int renoState;				// RENO状态机的状态，0为慢启动，1为拥塞控制，2为快速恢复

//协议头部
#pragma once(1)
struct Head {
	u_char type;						//数据包类型
	u_char window;						//窗口大小
	u_char seq;							//序列号
	u_short checksum;					//校验和
	u_short length;						//数据部分长度
	
	//构造函数
	Head() {
		type = 0;
		window = 0;
		seq = 0;
		checksum = 0;
		length = 0;
	}
};
#pragma once()

//函数声明
void init();
void release();
bool ShakeHand();
bool WaveHand();
u_short CheckSum(u_short* buffer, int length);

//数据包相关函数
//Clear head
void Clear(Head& head);
//Padding head
void Padding(Head& head, char* recvBuf);
//Packet head
void Packet(Head& head, u_char type, u_char seq, u_char window);

//发送数据包
void sendPackage(char* message, int length, u_char seq, u_char window, bool last);
//上传文件
void sendData(char* Buffer, int FileSize);
//读文件
void ReadFile();




//线程函数
DWORD WINAPI timer(LPVOID lparam);
DWORD WINAPI recvThread(LPVOID IpParameter);