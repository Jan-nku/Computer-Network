#ifndef _PACKET_H
#define _PACKET_H

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#pragma pack(1)
struct FrameHeader_t {									
	BYTE DesMAC[6];										//目的地址
	BYTE SrcMAC[6];										//源地址
	WORD FrameType;										//帧类型
};

struct ARPHeader_t {
	WORD HardwareType;									//硬件类型
	WORD ProtocolType;									//协议类型
	BYTE HLen;											//硬件地址长度
	BYTE PLen;											//协议地址
	WORD Operation;										//操作
	BYTE SendHa[6];										//发送方MAC
	DWORD SendIP;										//发送方IP
	BYTE RecvHa[6];										//接收方MAC
	DWORD RecvIP;										//接收方IP
};

struct IPHeader_t {
	BYTE Ver_HLen;
	BYTE TOS;
	WORD TotalLen;
	WORD ID;
	WORD Flag_Segment;
	BYTE TTL;											//生命周期
	BYTE Protocol;										//协议
	WORD Checksum;										//校验和
	DWORD SrcIP;										//源IP
	DWORD DstIP;										//目的IP
};

struct ICMPHeader_t {
	WORD Type;											//类型
	WORD Code;											//代码
	WORD Checksum;										//校验和
	WORD Id;											//标识符
	WORD Sequence;										//序号
};

struct ARP_t {
	FrameHeader_t FrameHeader;								//帧首部
	ARPHeader_t ARPHeader;									//ARP首部
};

struct IP_t {
	FrameHeader_t FrameHeader;							//帧首部
	IPHeader_t IPHeader;								//IP首部
};

struct ICMP_t {
	FrameHeader_t FrameHeader;
	IPHeader_t IPHeader;
	ICMPHeader_t ICMPHeader;
	char buf[50];
};
#pragma pack()

#define BROADMAC {0xff,0xff,0xff,0xff,0xff,0xff}		//广播MAC
#define EH_TYPE  0x0806									//ARP类型
#define ARP_HRD 0x0001									//硬件类型，以太网接口类型为1
#define ARP_PRO 0x0800									//协议类型：IP协议
#define ARP_HLN	0x06									//硬件地址长度
#define ARP_PLN 0x04									//协议地址长度
#define ARP_REQUEST 0x0001								//操作，ARP请求
#define ARP_REPLY 0x0002								//操作，ARP响应
#define ARP_THA {0,0,0,0,0,0}							//目的MAC地址，ARP请求中该字段没有意义

//广播ARP包帧头
#define ETH_HRD_DEFAULT {BROADMAC, {0x0f,0x0f,0x0f,0x0f,0x0f,0x0f}, htons(EH_TYPE)}
#define ARP_HRD_DEFAULT {htons(ARP_HRD), htons(ARP_PRO), ARP_HLN, ARP_PLN, htons(ARP_REQUEST), {0,0,0,0,0,0}, 0, ARP_THA, 0}


#endif // !_PACKET_H


