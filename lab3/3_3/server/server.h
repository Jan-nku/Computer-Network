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
#define Port 6666					//Server绑定端口
#define IP "127.0.0.1"				//Server绑定IP

#define MAX_WAIT_TIME 500			//最大等待时间500ms
#define MAX_SEND_SIZE 10000			//单次最大发送数据量10000Byte

//状态位
#define OVER 0x10
#define SEND 0x08
#define SYN 0x04
#define FIN 0x02
#define ACK 0x01

//声明全局变量
//Socket通讯
extern WSADATA wsaData;
extern SOCKET Server;
extern SOCKADDR_IN ServerAddr, ClientAddr;

//序列号
extern u_char Seq;


//接收数据
extern char FileName[20];			//文件名
extern u_int FileSize;				//文件大小
extern char Buffer[100000000];		//文件缓冲区
extern char recvBuf[20000];			//数据包接收缓冲区

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


//函数定义
void init();
void release();
bool ShakeHand();
bool WaveHand();
u_short CheckSum(u_short* buffer, int length);

//接收数据
void recvData();

//数据包相关函数
//Clear head
void Clear(Head& head);
//Padding head
void Padding(Head& head, char* recvBuf);
//Packet head
void Packet(Head& head, u_char type, u_char seq);



