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
#define Port 6665					//������(Router)�˿�
#define IP "127.0.0.1"				//������(Router)IP

#define MAX_WAIT_TIME 100			//���ȴ�ʱ��100ms
#define MAX_SEND_SIZE 10000			//���������������10000Byte
#define WINDOW 10					//����ͨ�洰�ڴ�С���̶�ֵ10

//״̬λ
#define OVER 0x10
#define SEND 0x08
#define SYN 0x04
#define FIN 0x02
#define ACK 0x01

//����ȫ�ֱ���
//SocketͨѶ
extern WSAData wsaData;
extern SOCKET Client;
extern SOCKADDR_IN ServerAddr;

//���к�
extern u_char Seq;

//�ļ���д
extern string FileName;				//�ļ���
extern u_int FileSize;				//�ļ���С
extern char Buffer[100000000];		//�ļ�������
extern char sendBuf[20000];			//�������ݻ�����

//GBN��Ҫʹ�õ�ȫ�ֱ���
extern int resendCount;				// �ش�����
extern int base;					// base֮ǰ���к��ۼ�ȷ��
extern u_char nextSeq;				// Ҫ���͵���һ�����к�
extern bool resend;					// �Ƿ�ʱ�ش�
extern bool restart;				// �Ƿ����¿�ʼ��ʱ
extern bool wait;					// �Ƿ��򴰿ڲ�������ȴ�
extern long long lenCopy;			// �ļ�����ƫ�����Ŀ��������ش�ʱʹ��

//ӵ������
extern const int rwnd;				// ����ͨ�洰�ڴ�С���̶�ֵ
extern double cwnd;					// ӵ�����ڴ�С
extern int ssthresh;				// ��ֵ
extern unsigned char lastAck;		// ��һ��ACK���к�
extern int dupAck;					// �ظ��յ���ACK����
extern int renoState;				// RENO״̬����״̬��0Ϊ��������1Ϊӵ�����ƣ�2Ϊ���ٻָ�

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

//���ݰ���غ���
//Clear head
void Clear(Head& head);
//Padding head
void Padding(Head& head, char* recvBuf);
//Packet head
void Packet(Head& head, u_char type, u_char seq, u_char window);

//�������ݰ�
void sendPackage(char* message, int length, u_char seq, u_char window, bool last);
//�ϴ��ļ�
void sendData(char* Buffer, int FileSize);
//���ļ�
void ReadFile();




//�̺߳���
DWORD WINAPI timer(LPVOID lparam);
DWORD WINAPI recvThread(LPVOID IpParameter);