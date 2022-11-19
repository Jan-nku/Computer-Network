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

char* myBroad;//�����㲥��ַ
unsigned char* m_MAC = new unsigned char[6];//����MAC��ַ
char* m_IP;//����IP��ַ
char* m_mask;//������������
#define IPTOSBUFFERS    12
#pragma pack(1)
struct ethernet_head {
	//����֡֡ͷ�ṹ
	unsigned char dst_mac[6];
	unsigned char src_mac[6];
	unsigned short eh_type;//��̫������(2�ֽ�)
};
struct arp_head {
	//ARP����֡
	unsigned short hardware_type;//Ӳ������
	unsigned short protocol_type;//Э������
	unsigned char add_len;//Ӳ����ַ����
	unsigned char pro_len;//Э���ַ����
	unsigned short option;//����������Ϊ1����ӦΪ2
	unsigned char src_mac[6];
	unsigned long src_ip;
	unsigned char dst_mac[6];
	unsigned long dst_ip;
	unsigned char padding[18];//���
};
struct arp_packet {
	//����arp���ṹ
	ethernet_head eth;
	arp_head arp;
};
#pragma pack()

//�̲߳���
struct sparam {
	pcap_t* adhandle;
	char* ip;
	unsigned char* mac;
	char* netmask;
};
struct gparam {
	pcap_t* adhandle;
};
struct sparam sp;//�����̲߳���
struct gparam gp;//�����̲߳���

#define BROADMAC {0xff,0xff,0xff,0xff,0xff,0xff}	//�㲥MAC
#define EH_TYPE  0x0806	//ARP����
#define ARP_HRD 0x0001	//Ӳ�����ͣ���̫���ӿ�����Ϊ1
#define ARP_PRO 0x0800	//Э�����ͣ�IPЭ��
#define ARP_HLN	0x06	//Ӳ����ַ����
#define ARP_PLN 0x04	//Э���ַ����
#define ARP_REQUEST 0x0001	//������ARP����
#define ARP_REPLY 0x0002	//������ARP��Ӧ
#define ARP_THA {0,0,0,0,0,0}	//Ŀ��MAC��ַ��ARP�����и��ֶ�û������
#define ARP_PAD {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}	//���
//�㲥ARP��֡ͷ
#define ETH_HRD_DEFAULT {BROADMAC, {0x0f,0x0f,0x0f,0x0f,0x0f,0x0f}, htons(EH_TYPE)}
#define ARP_HRD_DEFAULT {htons(ARP_HRD), htons(ARP_PRO), ARP_HLN, ARP_PLN, htons(ARP_REQUEST), {0,0,0,0,0,0}, 0, ARP_THA, 0, ARP_PAD}

void PrintAlldevs();
void GetSelfMac(pcap_t* adhandle, const char* ip_addr, unsigned char* ip_mac);	// ��ȡ������MAC��ַ
void sendArpRequest(pcap_t* adhandle, const char* ip, unsigned char* mac);//����ARP���ݰ�
char* iptos(u_long in)//�������������ת������
{
	static char output[IPTOSBUFFERS][3 * 4 + 3 + 1];
	static short which;
	u_char* p;

	p = (u_char*)&in;
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
	sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return output[which];
}
void ifprint(pcap_if_t* d);//��ӡ������Ϣ
DWORD WINAPI SendArpPacket(LPVOID lpParameter);//����ARP�߳�
DWORD WINAPI GetLivePC(LPVOID lpParameter);//���ջظ��߳�

int main() {
	PrintAlldevs();
	if (pcap_findalldevs(&alldevs, errbuf) == -1) {//��ȡ�豸�б�
		//������
	}
	int nChoose;
	cin >> nChoose;
	for (int x = 0; x < nChoose - 1; ++x) { //�ҵ�ָ��������
		alldevs = alldevs->next;
		for (a = alldevs->addresses; a != nullptr; a = a->next) {
			if (a->addr->sa_family == AF_INET) { //�жϸõ�ַ�Ƿ�IP��ַ
				ip_addr = inet_ntoa(((sockaddr_in*)a->addr)->sin_addr);
			}
		}
	}
	pcap_t* adhandle = pcap_open(alldevs->name, 65534, PCAP_OPENFLAG_PROMISCUOUS, 1, 0, 0);
	GetSelfMac(adhandle, ip_addr, ip_mac);
	printf("����mac��ַ:");
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

	//HANDLE sendthread;      //����ARP���߳�
	//HANDLE recvthread;       //����ARP���߳�
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

	pcap_freealldevs(alldevs);//�ͷ������������б�
}


void GetSelfMac(pcap_t* adhandle, const char* ip_addr, unsigned char* ip_mac) {
	//���췢�����ݰ�
	arp_packet arpPacket;

	arpPacket.eth = ETH_HRD_DEFAULT;//��̫��֡ͷ
	arpPacket.arp = ARP_HRD_DEFAULT;//ARP֡ͷ
	arpPacket.arp.src_ip = inet_addr("100.100.100.100");
	arpPacket.arp.dst_ip = inet_addr(ip_addr);
	if (pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60) != 0) {
		//������
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
			printf("��ȡ�Լ�������MAC��ַ�ɹ�!\n");
			break;
		}
	}

}
void sendArpRequest(pcap_t* adhandle, const char* ip, unsigned char* mac) {
	arp_packet arpPacket;
	arpPacket.eth = ETH_HRD_DEFAULT;//��̫��֡ͷ
	arpPacket.arp = ARP_HRD_DEFAULT;//ARP֡ͷ
	arpPacket.arp.src_ip = inet_addr(ip_addr);
	arpPacket.arp.dst_ip = inet_addr(ip);
	memcpy(arpPacket.eth.src_mac, ip_mac, 6);
	memcpy(arpPacket.arp.src_mac, ip_mac, 6);
	if (pcap_sendpacket(adhandle, (const u_char*)&arpPacket, 60) != 0) {
		//������
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
	cout << "�����������Ϣ" << endl;
	// ��ȡ���ػ����豸�б�
	if (pcap_findalldevs_ex((char*)PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) != -1)
	{
		/* ��ӡ������Ϣ�б� */
		for (d = alldevs; d != NULL; d = d->next)
		{
			++index;
			if (d->description)
				printf("%s ID: %d --> Name: %s \n", d->name, index, d->description);
			ifprint(d);
			for (a = d->addresses; a != nullptr; a = a->next) {
				sockaddr_in* tmp;
				u_char b1, b2, b3, b4;
				if (a->addr->sa_family == AF_INET) { //�жϸõ�ַ�Ƿ�IP��ַ
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

/* ��ӡ���п�����Ϣ */
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
	printf("�����IP��ַΪ:%s\n", ip);
	printf("��ַ����NETMASKΪ:%s\n", netmask);
	printf("\n");
	unsigned char sendbuf[42]; //arp���ṹ��С
	ethernet_head eh;
	arp_head ah;
	//��ֵMAC��ַ
	memset(eh.dst_mac, 0xff, 6);       //Ŀ�ĵ�ַΪȫΪ�㲥��ַ
	memcpy(eh.src_mac, mac, 6);
	memcpy(ah.src_mac, mac, 6);
	memset(ah.dst_mac, 0x00, 6);
	eh.eh_type = htons(EH_TYPE);//֡����ΪARP3
	ah.hardware_type = htons(ARP_HRD);
	ah.protocol_type = htons(ARP_PRO);
	ah.add_len = 6;
	ah.pro_len = 4;
	ah.src_ip = inet_addr(ip); //���󷽵�IP��ַΪ�����IP��ַ
	ah.option = htons(ARP_REQUEST);
	//��������ڹ㲥����arp��
	unsigned long myip = inet_addr(ip);
	unsigned long mynetmask = inet_addr(netmask);
	unsigned long hisip = htonl((myip & mynetmask));
	//��ָ��IP��������
	char desIP[16];
	printf("����Ŀ��IP:");
	//scanf("%s", &desIP);
	memcpy(desIP, "121.43.173.240", 16);
	ah.dst_ip = (inet_addr(desIP));
	//����һ��ARP����
	memset(sendbuf, 0, sizeof(sendbuf));
	memcpy(sendbuf, &eh, sizeof(eh));
	memcpy(sendbuf + sizeof(eh), &ah, sizeof(ah));
	//������ͳɹ�
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
					printf("IP��ַ:%d.%d.%d.%d   MAC��ַ:",
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
