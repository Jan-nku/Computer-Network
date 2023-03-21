#ifndef _Function_H
#define _Function_H

#include"Packet.h"
#include"Table.h"
#include<iostream>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#define HAVE_REMOTE
#include "pcap.h"
#define WIN32
using namespace std;

extern pcap_if_t* alldevs;
extern pcap_if_t* d;
extern pcap_addr_t* a;
extern char errbuf[PCAP_ERRBUF_SIZE];
extern struct pcap_pkthdr* pkt_header;
extern const u_char* pkt_data;
extern pcap_t* adhandle;

bool compare(BYTE mac1[], BYTE mac2[]);										//�Ƚ�mac��ַ
bool CheckSum(IP_t* ip_packet);												//����checksum
void setCheckSum(IP_t* ip_packet);											//����checksum
void printMac(BYTE mac[]);													//��ӡmac��ַ
void printIp(DWORD ip);														//��ӡip
void printAlldevs();														//��ӡ�豸��Ϣ
void printIf(pcap_if_t* d);													//��ӡ������Ϣ
pcap_t* open(int choose);													//�������豸����ȡIP�����롢MAC��ַ
DWORD getNet(DWORD ip, DWORD mask);											//��ȡ�����
void getSelfMac(pcap_t* adhandle, DWORD ip, BYTE* mac);						//��ƭ��ȡ����MAC
void getOtherMac(pcap_t* adhandle, DWORD ip_, BYTE* mac_);					//��ȡIP��ӦMAC
void sendARP(pcap_t* adhandle, DWORD ip_);									//����ARP���ݰ�
void resend(pcap_t* adhandle, ICMP_t data, DWORD nextip, BYTE dstMac[]);	//ת�����ݰ�
DWORD WINAPI handlerRequest(LPVOID lparam);									//·��ת���߳�
int find(Buffer* buffer, int size);											//���һ������п���λ��
void handle(Buffer* buffer, int size);										//��������
void cache(Buffer* buffer, int size, ICMP_t* data, DWORD TargetIP);			//�������ݰ�
void ICMPPacket(BYTE type, BYTE code, const u_char* pkt_data);				//����ICMP����


#endif // !_Function_H
