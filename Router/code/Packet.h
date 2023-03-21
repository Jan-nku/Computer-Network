#ifndef _PACKET_H
#define _PACKET_H

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#pragma pack(1)
struct FrameHeader_t {									
	BYTE DesMAC[6];										//Ŀ�ĵ�ַ
	BYTE SrcMAC[6];										//Դ��ַ
	WORD FrameType;										//֡����
};

struct ARPHeader_t {
	WORD HardwareType;									//Ӳ������
	WORD ProtocolType;									//Э������
	BYTE HLen;											//Ӳ����ַ����
	BYTE PLen;											//Э���ַ
	WORD Operation;										//����
	BYTE SendHa[6];										//���ͷ�MAC
	DWORD SendIP;										//���ͷ�IP
	BYTE RecvHa[6];										//���շ�MAC
	DWORD RecvIP;										//���շ�IP
};

struct IPHeader_t {
	BYTE Ver_HLen;
	BYTE TOS;
	WORD TotalLen;
	WORD ID;
	WORD Flag_Segment;
	BYTE TTL;											//��������
	BYTE Protocol;										//Э��
	WORD Checksum;										//У���
	DWORD SrcIP;										//ԴIP
	DWORD DstIP;										//Ŀ��IP
};

struct ICMPHeader_t {
	WORD Type;											//����
	WORD Code;											//����
	WORD Checksum;										//У���
	WORD Id;											//��ʶ��
	WORD Sequence;										//���
};

struct ARP_t {
	FrameHeader_t FrameHeader;								//֡�ײ�
	ARPHeader_t ARPHeader;									//ARP�ײ�
};

struct IP_t {
	FrameHeader_t FrameHeader;							//֡�ײ�
	IPHeader_t IPHeader;								//IP�ײ�
};

struct ICMP_t {
	FrameHeader_t FrameHeader;
	IPHeader_t IPHeader;
	ICMPHeader_t ICMPHeader;
	char buf[50];
};
#pragma pack()

#define BROADMAC {0xff,0xff,0xff,0xff,0xff,0xff}		//�㲥MAC
#define EH_TYPE  0x0806									//ARP����
#define ARP_HRD 0x0001									//Ӳ�����ͣ���̫���ӿ�����Ϊ1
#define ARP_PRO 0x0800									//Э�����ͣ�IPЭ��
#define ARP_HLN	0x06									//Ӳ����ַ����
#define ARP_PLN 0x04									//Э���ַ����
#define ARP_REQUEST 0x0001								//������ARP����
#define ARP_REPLY 0x0002								//������ARP��Ӧ
#define ARP_THA {0,0,0,0,0,0}							//Ŀ��MAC��ַ��ARP�����и��ֶ�û������

//�㲥ARP��֡ͷ
#define ETH_HRD_DEFAULT {BROADMAC, {0x0f,0x0f,0x0f,0x0f,0x0f,0x0f}, htons(EH_TYPE)}
#define ARP_HRD_DEFAULT {htons(ARP_HRD), htons(ARP_PRO), ARP_HLN, ARP_PLN, htons(ARP_REQUEST), {0,0,0,0,0,0}, 0, ARP_THA, 0}


#endif // !_PACKET_H


