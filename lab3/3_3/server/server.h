#pragma once
//���ļ�
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

//�궨��
#define Port 6666					//Server�󶨶˿�
#define IP "127.0.0.1"				//Server��IP

#define MAX_WAIT_TIME 500			//���ȴ�ʱ��500ms
#define MAX_SEND_SIZE 10000			//���������������10000Byte

//״̬λ
#define OVER 0x10
#define SEND 0x08
#define SYN 0x04
#define FIN 0x02
#define ACK 0x01

//����ȫ�ֱ���
//SocketͨѶ
extern WSADATA wsaData;
extern SOCKET Server;
extern SOCKADDR_IN ServerAddr, ClientAddr;

//���к�
extern u_char Seq;


//��������
extern char FileName[20];			//�ļ���
extern u_int FileSize;				//�ļ���С
extern char Buffer[100000000];		//�ļ�������
extern char recvBuf[20000];			//���ݰ����ջ�����

//Э��ͷ��
#pragma once(1)
struct Head {
	u_char type;						//���ݰ�����
	u_char window;						//���ڴ�С
	u_char seq;							//���к�
	u_short checksum;					//У���
	u_short length;						//���ݲ��ֳ���

	//���캯��
	Head() {
		type = 0;
		window = 0;
		seq = 0;
		checksum = 0;
		length = 0;
	}
};
#pragma once()


//��������
void init();
void release();
bool ShakeHand();
bool WaveHand();
u_short CheckSum(u_short* buffer, int length);

//��������
void recvData();

//���ݰ���غ���
//Clear head
void Clear(Head& head);
//Padding head
void Padding(Head& head, char* recvBuf);
//Packet head
void Packet(Head& head, u_char type, u_char seq);



