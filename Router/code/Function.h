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

bool compare(BYTE mac1[], BYTE mac2[]);										//比较mac地址
bool CheckSum(IP_t* ip_packet);												//检验checksum
void setCheckSum(IP_t* ip_packet);											//设置checksum
void printMac(BYTE mac[]);													//打印mac地址
void printIp(DWORD ip);														//打印ip
void printAlldevs();														//打印设备信息
void printIf(pcap_if_t* d);													//打印网卡信息
pcap_t* open(int choose);													//打开网卡设备，获取IP、掩码、MAC地址
DWORD getNet(DWORD ip, DWORD mask);											//获取网络号
void getSelfMac(pcap_t* adhandle, DWORD ip, BYTE* mac);						//欺骗获取网卡MAC
void getOtherMac(pcap_t* adhandle, DWORD ip_, BYTE* mac_);					//获取IP对应MAC
void sendARP(pcap_t* adhandle, DWORD ip_);									//发送ARP数据包
void resend(pcap_t* adhandle, ICMP_t data, DWORD nextip, BYTE dstMac[]);	//转发数据包
DWORD WINAPI handlerRequest(LPVOID lparam);									//路由转发线程
int find(Buffer* buffer, int size);											//查找缓冲区中空余位置
void handle(Buffer* buffer, int size);										//处理缓冲区
void cache(Buffer* buffer, int size, ICMP_t* data, DWORD TargetIP);			//缓存数据包
void ICMPPacket(BYTE type, BYTE code, const u_char* pkt_data);				//发送ICMP报文


#endif // !_Function_H
