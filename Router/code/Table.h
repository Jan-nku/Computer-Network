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
	DWORD mask;									//掩码
	DWORD net;									//目的网络
	DWORD nextip;								//下一跳
	int type;									//类型
	routeItem* nextItem;						//链表形式存储
	routeItem() {
		memset(this, 0, sizeof(*this));
	}
	routeItem(char* mask, char* net, char* nextip) {
		this->mask = inet_addr(mask);
		this->net = inet_addr(net);
		this->nextip = inet_addr(nextip);
		this->type = 1;
	}
	void printItem();							//打印掩码、目的网络、下一跳IP和类型
};

class routeTable {
public:
	routeItem* head, * tail;
	int num;
	routeTable();								//路由表的初始化，添加直接投递项
	void add(routeItem* item);					//路由表的添加，直接投递在最前，前缀长的在前面
	void remove(int index);						//路由表的删除，直接投递不可删除
	void printTable();							//路由表的打印：mask、net、nextip、type
	DWORD lookup(DWORD ip);						//查找最长前缀，返回下一跳的ip
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
	BYTE Data[DataSize];								//数据部分
	WORD TargetIP;								//目的IP
	bool valid = 1;								//有效位，转发或超时置0
	clock_t clock;								//超时判断
};
extern Buffer buffer[BufferSize];


#endif // !_TABLE_H
