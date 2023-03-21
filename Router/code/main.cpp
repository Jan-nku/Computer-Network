#include"Packet.h"
#include"Table.h"
#include"Function.h"
#include<iostream>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#define HAVE_REMOTE
#include "pcap.h"
#define WIN32
using namespace std;


int main() {
	printAlldevs();
	adhandle = open(1);
	getSelfMac(adhandle, ip[0], mac);
	printMac(mac);
	printf("\n");
	char* input_ip = new char[16];
	char* input_mask = new char[16];
	char* input_net = new char[16];
	DWORD ip_;
	BYTE mac_[6];
	routeTable routetable;
	int op, index;

	printf("请完成初始化路由表工作\n");
	printf("添加路由表项条数：");

	cin >> index;
	do {
		printf("掩码：");
		cin >> input_mask;
		printf("目的网络：");
		cin >> input_net;
		printf("下一跳：");
		cin >> input_ip;
		routetable.add(new routeItem(input_mask, input_net, input_ip));
		printf("添加成功!\n");
	} while (--index);
	printf("当前路由表\n");
	routetable.printTable();
	printf("路由功能正在启动...\n");
	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, NULL, handlerRequest, LPVOID(&routetable), 0, &dwThreadId);
	printf("路由功能已经启动！\n");

	while (1) {
		printf("=======程序功能======\n");
		printf("\t1:增加路由表项\n");
		printf("\t2:删除路由表项\n");
		printf("\t3:打印路由表\n");
		printf("\t4:ARP查询\n");
		printf("\t5:打印ARP表\n");
		printf("请输入要进行的操作：\n");
		cin >> op;
		switch (op) {
		case 1:
			printf("掩码：");
			cin >> input_mask;
			printf("目的网络：");
			cin >> input_net;
			printf("下一跳：");
			cin >> input_ip;
			routetable.add(new routeItem(input_mask, input_net, input_ip));
			break;
		case 2:
			routetable.printTable();
			printf("请选择要删除的表项index：");
			cin >> index;
			routetable.remove(index);
			break;
		case 3:
			routetable.printTable();
			break;
		case 4:
			printf("请输入要查询的IP地址：");
			cin >> input_ip;
			ip_ = inet_addr(input_ip);
			getOtherMac(adhandle, ip_, mac_);
			printMac(mac_);
			printf("\n");
			break;
		case 5:
			arpTable::printArp();
			break;
		default:
			printf("输入异常，程序终止\n");
		}
	}
	CloseHandle(hThread);
}
