#include<iostream>
#define HAVE_REMOTE
#define WIN32
#include<WinSock2.h>
#include "pcap.h"
#pragma comment(lib,"ws2_32.lib")
using namespace std;

pcap_if_t* alldevs;
pcap_if_t* d;
pcap_addr_t* a;
const char* ip_addr;
unsigned char* ip_mac = new unsigned char[6];
char errbuf[PCAP_ERRBUF_SIZE];
struct pcap_pkthdr* pkt_header;
const u_char* pkt_data;

char* myBroad;//本机广播地址
unsigned char* m_MAC = new unsigned char[6];//本机MAC地址
char* m_IP;//本机IP地址
char* m_mask;//本机网络掩码
#define IPTOSBUFFERS    12
#pragma pack(1)
struct ethernet_head {
	//物理帧帧头结构
	unsigned char dst_mac[6];
	unsigned char src_mac[6];
	unsigned short eh_type;//以太网类型(2字节)
};
struct arp_head {
	//ARP数据帧
	unsigned short hardware_type;//硬件类型
	unsigned short protocol_type;//协议类型
	unsigned char add_len;//硬件地址长度
	unsigned char pro_len;//协议地址长度
	unsigned short option;//操作，请求为1，响应为2
	unsigned char src_mac[6];
	unsigned long src_ip;
	unsigned char dst_mac[6];
	unsigned long dst_ip;
	unsigned char padding[18];//填充
};
struct arp_packet {
	//最终arp包结构
	ethernet_head eth;
	arp_head arp;
};
#pragma pack()

//线程参数
struct sparam {
	pcap_t* adhandle;
	char* ip;
	unsigned char* mac;
	char* netmask;
};
struct gparam {
	pcap_t* adhandle;
};
struct sparam sp;//发送线程参数
struct gparam gp;//接收线程参数

#define BROADMAC {0xff,0xff,0xff,0xff,0xff,0xff}	//广播MAC
#define EH_TYPE  0x0806	//ARP类型
#define ARP_HRD 0x0001	//硬件类型，以太网接口类型为1
#define ARP_PRO 0x0800	//协议类型：IP协议
#define ARP_HLN	0x06	//硬件地址长度
#define ARP_PLN 0x04	//协议地址长度
#define ARP_REQUEST 0x0001	//操作，ARP请求
#define ARP_REPLY 0x0002	//操作，ARP响应
#define ARP_THA {0,0,0,0,0,0}	//目的MAC地址，ARP请求中该字段没有意义
#define ARP_PAD {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}	//填充
//广播ARP包帧头
#define ETH_HRD_DEFAULT {BROADMAC, {0x0f,0x0f,0x0f,0x0f,0x0f,0x0f}, htons(EH_TYPE)}
#define ARP_HRD_DEFAULT {htons(ARP_HRD), htons(ARP_PRO), ARP_HLN, ARP_PLN, htons(ARP_REQUEST), {0,0,0,0,0,0}, 0, ARP_THA, 0, ARP_PAD}

void PrintAlldevs();
void GetSelfMac(pcap_t* adhandle, const char* ip_addr, unsigned char* ip_mac);	// 获取主机的MAC地址
void sendArpRequest(pcap_t* adhandle, const char* ip, unsigned char* mac);//发送ARP数据包
char* iptos(u_long in)//网络序和主机序转换函数
{
	static char output[IPTOSBUFFERS][3 * 4 + 3 + 1];
	static short which;
	u_char* p;

	p = (u_char*)&in;
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
	sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return output[which];
}
void ifprint(pcap_if_t* d);//打印网卡信息
DWORD WINAPI SendArpPacket(LPVOID lpParameter);//发送ARP线程
DWORD WINAPI GetLivePC(LPVOID lpParameter);//接收回复线程

int main() {
	PrintAlldevs();
	if (pcap_findalldevs(&alldevs, errbuf) == -1) {//获取设备列表
		//错误处理
	}
	int nChoose;
	cin >> nChoose;
	for (int x = 0; x < nChoose - 1; ++x) { //找到指定的网卡
		alldevs = alldevs->next;
		for (a = alldevs->addresses; a != nullptr; a = a->next) {
			if (a->addr->sa_family == AF_INET) { //判断该地址是否IP地址
				ip_addr = inet_ntoa(((sockaddr_in*)a->addr)->sin_addr);
			}
		}
	}
	pcap_t* adhandle = pcap_open(alldevs->name, 65534, PCAP_OPENFLAG_PROMISCUOUS, 1, 0, 0);
	GetSelfMac(adhandle, ip_addr, ip_mac);
	printf("本机mac地址:");
	for (int i = 0; i < 6; i++) {
		if (i != 5)
			printf("%02x.", ip_mac[i]);
		else
			printf("%02x\n", ip_mac[i]);
	}

	char* ip = new char[16];
	unsigned char* mac = new unsigned char[6];
	while (1) {
		cin >> ip;
		sendArpRequest(adhandle, ip, mac);
	}

	//HANDLE sendthread;      //发送ARP包线程
	//HANDLE recvthread;       //接受ARP包线程
	//sp.adhandle = adhandle;
	//sp.ip = m_IP;
	//sp.netmask = m_mask;
	//sp.mac = m_MAC;
	//gp.adhandle = adhandle;

	//sendthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendArpPacket, &sp, 0, NULL);
	//recvthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetLivePC, &gp, 0, NULL);

	//pcap_freealldevs(alldevs);
	//CloseHandle(sendthread);
	//CloseHandle(recvthread);
	//while (1);
	//return 0;

	pcap_freealldevs(alldevs);//释放网络适配器列表
}


void GetSelfMac(pcap_t* adhandle, const char* ip_addr, unsigned char* ip_mac) {
	//构造发送数据包
	arp_packet arpPacket;

	arpPacket.eth = ETH_HRD_DEFAULT;//以太网帧头
	arpPacket.arp = ARP_HRD_DEFAULT;//ARP帧头
	arpPacket.arp.src_ip = inet_addr("100.100.100.100");
	arpPacket.arp.dst_ip = inet_addr(ip_addr);
	if (pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60) != 0) {
		//错误处理
	}
	int i, res;
	while ((res = pcap_next_ex(adhandle, &pkt_header, &pkt_data)) >= 0) {
		if (res == 0)continue;
		if (*(unsigned short*)(pkt_data + 12) == htons(EH_TYPE)
			&& *(unsigned short*)(pkt_data + 20) == htons(ARP_REPLY)
			&& *(unsigned long*)(pkt_data + 38)
			== inet_addr("100.100.100.100")) {
			for (i = 0; i < 6; i++) {
				ip_mac[i] = *(unsigned char*)(pkt_data + 22 + i);
			}
			printf("获取自己主机的MAC地址成功!\n");
			break;
		}
	}

}
void sendArpRequest(pcap_t* adhandle, const char* ip, unsigned char* mac) {
	arp_packet arpPacket;
	arpPacket.eth = ETH_HRD_DEFAULT;//以太网帧头
	arpPacket.arp = ARP_HRD_DEFAULT;//ARP帧头
	arpPacket.arp.src_ip = inet_addr(ip_addr);
	arpPacket.arp.dst_ip = inet_addr(ip);
	memcpy(arpPacket.eth.src_mac, ip_mac, 6);
	memcpy(arpPacket.arp.src_mac, ip_mac, 6);
	if (pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60) != 0) {
		//错误处理
	}
	pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60);
	pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60);
	pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60);
	pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60);
	int i, res;
	while ((res = pcap_next_ex(adhandle, &pkt_header, &pkt_data)) >= 0) {
		if (res == 0)continue;
		if (*(unsigned short*)(pkt_data + 12) == htons(EH_TYPE)
			&& *(unsigned short*)(pkt_data + 20) == htons(ARP_REPLY)
			&& *(unsigned long*)(pkt_data + 38)
			== inet_addr(ip_addr)) {
			for (i = 0; i < 6; i++) {
				mac[i] = *(unsigned char*)(pkt_data + 22 + i);
			}
			for (int i = 0; i < 6; i++) {
				if (i != 5)
					printf("%02x.", mac[i]);
				else
					printf("%02x\n", mac[i]);
			}
			break;
		}
	}
}
void PrintAlldevs() {
	int index = 0;
	cout << "输出网卡的信息" << endl;
	// 获取本地机器设备列表
	if (pcap_findalldevs_ex((char*)PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) != -1)
	{
		/* 打印网卡信息列表 */
		for (d = alldevs; d != NULL; d = d->next)
		{
			++index;
			if (d->description)
				printf("%s ID: %d --> Name: %s \n", d->name, index, d->description);
			ifprint(d);
			for (a = d->addresses; a != nullptr; a = a->next) {
				sockaddr_in* tmp;
				u_char b1, b2, b3, b4;
				if (a->addr->sa_family == AF_INET) { //判断该地址是否IP地址
					tmp = (sockaddr_in*)(a->addr);
					b1 = tmp->sin_addr.S_un.S_un_b.s_b1;
					b2 = tmp->sin_addr.S_un.S_un_b.s_b2;
					b3 = tmp->sin_addr.S_un.S_un_b.s_b3;
					b4 = tmp->sin_addr.S_un.S_un_b.s_b4;
					if (b1 != 0) {
						printf(" addr: %d.%d.%d.%d\n", b1, b2, b3, b4);
					}
				}
			}
		}
	}
	pcap_freealldevs(alldevs);
}

/* 打印所有可用信息 */
void ifprint(pcap_if_t* d)
{
	pcap_addr_t* a;

	/* IP addresses */
	for (a = d->addresses; a; a = a->next)
	{
		printf("  Address Family: #%d\n", a->addr->sa_family);
		switch (a->addr->sa_family)
		{
		case AF_INET:
			printf("  Address Family Name: AF_INET\n");
			if (a->addr)
			{
				m_IP = inet_ntoa(((struct sockaddr_in*)a->addr)->sin_addr);
				printf("  IP Address: %s\n", m_IP);
			}
			if (a->netmask)
			{
				m_mask = inet_ntoa(((struct sockaddr_in*)a->netmask)->sin_addr);
				printf("  Netmask: %s\n", m_mask);
			}

			if (a->broadaddr)
			{
				myBroad = inet_ntoa(((sockaddr_in*)a->broadaddr)->sin_addr);
				printf("  Broadcast Address: %s\n", myBroad);
			}
			if (a->dstaddr)
				printf("  Destination Address: %s\n", inet_ntoa(((struct sockaddr_in*)a->dstaddr)->sin_addr));
			break;
		default:
			printf("  Address Family Name: Unknown\n");
			break;
		}
	}
	printf("\n");
}

bool flag;
DWORD WINAPI SendArpPacket(LPVOID lpParameter)
{
	sparam* spara = (sparam*)lpParameter;
	pcap_t* adhandle = spara->adhandle;
	char* ip = spara->ip;
	unsigned char* mac = spara->mac;
	char* netmask = spara->netmask;
	printf("ip_mac:%02x-%02x-%02x-%02x-%02x-%02x\n", mac[0], mac[1], mac[2],
		mac[3], mac[4], mac[5]);
	printf("自身的IP地址为:%s\n", ip);
	printf("地址掩码NETMASK为:%s\n", netmask);
	printf("\n");
	unsigned char sendbuf[42]; //arp包结构大小
	ethernet_head eh;
	arp_head ah;
	//赋值MAC地址
	memset(eh.dst_mac, 0xff, 6);       //目的地址为全为广播地址
	memcpy(eh.src_mac, mac, 6);
	memcpy(ah.src_mac, mac, 6);
	memset(ah.dst_mac, 0x00, 6);
	eh.eh_type = htons(EH_TYPE);//帧类型为ARP3
	ah.hardware_type = htons(ARP_HRD);
	ah.protocol_type = htons(ARP_PRO);
	ah.add_len = 6;
	ah.pro_len = 4;
	ah.src_ip = inet_addr(ip); //请求方的IP地址为自身的IP地址
	ah.option = htons(ARP_REQUEST);
	//向局域网内广播发送arp包
	unsigned long myip = inet_addr(ip);
	unsigned long mynetmask = inet_addr(netmask);
	unsigned long hisip = htonl((myip & mynetmask));
	//向指定IP主机发送
	char desIP[16];
	printf("输入目标IP:");
	//scanf("%s", &desIP);
	memcpy(desIP, "121.43.173.240", 16);
	ah.dst_ip = (inet_addr(desIP));
	//构造一个ARP请求
	memset(sendbuf, 0, sizeof(sendbuf));
	memcpy(sendbuf, &eh, sizeof(eh));
	memcpy(sendbuf + sizeof(eh), &ah, sizeof(ah));
	//如果发送成功
	int i = 0;
	while (i<=10) {
		if (pcap_sendpacket(adhandle, sendbuf, 42) == 0) {
			printf("\nPacketSend succeed\n");
		}
		else {
			printf("PacketSendPacket in getmine Error: %d\n", GetLastError());
		}
		i++;
	}
	flag = TRUE;
	return 0;
}

DWORD WINAPI GetLivePC(LPVOID lpParameter)
{
	gparam* gpara = (gparam*)lpParameter;
	pcap_t* adhandle = gpara->adhandle;
	int res;
	unsigned char Mac[6];
	//struct pcap_pkthdr* pkt_header;
	//const u_char* pkt_data;
	while (true) {
		if ((res = pcap_next_ex(adhandle, &pkt_header, &pkt_data)) >= 0) {
			if (*(unsigned short*)(pkt_data + 12) == htons(EH_TYPE)) {
				arp_packet* recv = (arp_packet*)pkt_data;
				if (*(unsigned short*)(pkt_data + 20) == htons(ARP_REPLY)) {
					printf("-------------------------------------------\n");
					printf("IP地址:%d.%d.%d.%d   MAC地址:",
						recv->arp.src_ip & 255,
						recv->arp.src_ip >> 8 & 255,
						recv->arp.src_ip >> 16 & 255,
						recv->arp.src_ip >> 24 & 255);
					for (int i = 0; i < 6; i++) {
						Mac[i] = *(unsigned char*)(pkt_data + 22 + i);
						printf("%02x ", Mac[i]);
					}
					printf("\n");
				}
			}
		}
		Sleep(10);
	}
	return 0;
}
