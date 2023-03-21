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

	printf("����ɳ�ʼ��·�ɱ���\n");
	printf("���·�ɱ���������");

	cin >> index;
	do {
		printf("���룺");
		cin >> input_mask;
		printf("Ŀ�����磺");
		cin >> input_net;
		printf("��һ����");
		cin >> input_ip;
		routetable.add(new routeItem(input_mask, input_net, input_ip));
		printf("��ӳɹ�!\n");
	} while (--index);
	printf("��ǰ·�ɱ�\n");
	routetable.printTable();
	printf("·�ɹ�����������...\n");
	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, NULL, handlerRequest, LPVOID(&routetable), 0, &dwThreadId);
	printf("·�ɹ����Ѿ�������\n");

	while (1) {
		printf("=======������======\n");
		printf("\t1:����·�ɱ���\n");
		printf("\t2:ɾ��·�ɱ���\n");
		printf("\t3:��ӡ·�ɱ�\n");
		printf("\t4:ARP��ѯ\n");
		printf("\t5:��ӡARP��\n");
		printf("������Ҫ���еĲ�����\n");
		cin >> op;
		switch (op) {
		case 1:
			printf("���룺");
			cin >> input_mask;
			printf("Ŀ�����磺");
			cin >> input_net;
			printf("��һ����");
			cin >> input_ip;
			routetable.add(new routeItem(input_mask, input_net, input_ip));
			break;
		case 2:
			routetable.printTable();
			printf("��ѡ��Ҫɾ���ı���index��");
			cin >> index;
			routetable.remove(index);
			break;
		case 3:
			routetable.printTable();
			break;
		case 4:
			printf("������Ҫ��ѯ��IP��ַ��");
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
			printf("�����쳣��������ֹ\n");
		}
	}
	CloseHandle(hThread);
}
