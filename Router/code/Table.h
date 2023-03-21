#ifndef _TABLE_H
#define _TABLE_H

#include<WinSock2.h>
#include<iostream>
#include<ctime>
using namespace std;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

extern DWORD ip[2], mask[2];
extern BYTE mac[6];
#define BufferSize 10
#define DataSize 100
#define BufferTime 30000

class routeItem {
public:
	DWORD mask;									//����
	DWORD net;									//Ŀ������
	DWORD nextip;								//��һ��
	int type;									//����
	routeItem* nextItem;						//������ʽ�洢
	routeItem() {
		memset(this, 0, sizeof(*this));
	}
	routeItem(char* mask, char* net, char* nextip) {
		this->mask = inet_addr(mask);
		this->net = inet_addr(net);
		this->nextip = inet_addr(nextip);
		this->type = 1;
	}
	void printItem();							//��ӡ���롢Ŀ�����硢��һ��IP������
};

class routeTable {
public:
	routeItem* head, * tail;
	int num;
	routeTable();								//·�ɱ�ĳ�ʼ�������ֱ��Ͷ����
	void add(routeItem* item);					//·�ɱ����ӣ�ֱ��Ͷ������ǰ��ǰ׺������ǰ��
	void remove(int index);						//·�ɱ��ɾ����ֱ��Ͷ�ݲ���ɾ��
	void printTable();							//·�ɱ�Ĵ�ӡ��mask��net��nextip��type
	DWORD lookup(DWORD ip);						//�����ǰ׺��������һ����ip
	void printNum();
};

class arpTable {
public:
	DWORD ip;
	BYTE mac[6];
	static int num;
	static void insert(DWORD ip, BYTE mac[6]);
	static int lookup(DWORD ip, BYTE mac[6]);
	static void printArp();
};

extern arpTable arptable[50];

class Buffer {
public:
	BYTE Data[DataSize];								//���ݲ���
	WORD TargetIP;								//Ŀ��IP
	bool valid = 1;								//��Чλ��ת����ʱ��0
	clock_t clock;								//��ʱ�ж�
};
extern Buffer buffer[BufferSize];


#endif // !_TABLE_H
