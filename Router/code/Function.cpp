#include"Function.h"

pcap_if_t* alldevs;
pcap_if_t* d;
pcap_addr_t* a;
char errbuf[PCAP_ERRBUF_SIZE];
struct pcap_pkthdr* pkt_header;
const u_char* pkt_data;

pcap_t* adhandle;

bool compare(BYTE mac1[], BYTE mac2[]) {
	for (int i = 0; i < 6; i++) {
		if (mac1[i] != mac2[i]) {
			return false;
		}
	}
	return true;
}

bool CheckSum(IP_t* ip_packet) {
	unsigned int sum = 0;
	WORD* tmp = (WORD*)&ip_packet->IPHeader;
	for (int i = 0; i < sizeof(IPHeader_t) / 2; i++) {
		sum += tmp[i];
		if (sum & 0xffff0000) {
			sum &= 0xffff;
			sum++;
		}
	}
	if (sum == 0xffff) {
		return 1;
	}
	return 0;
}

void setCheckSum(IP_t* ip_packet)
{
	ip_packet->IPHeader.Checksum = 0;
	unsigned int sum = 0;
	WORD* tmp = (WORD*)&ip_packet->IPHeader;
	for (int i = 0; i < sizeof(IPHeader_t) / 2; i++)
	{
		sum += tmp[i];
		if (sum & 0xffff0000) {
			sum &= 0xffff;
			sum++;
		}
	}
	ip_packet->IPHeader.Checksum = ~sum;
}

void printMac(BYTE mac[]) {
	for (int i = 0; i < 5; i++) {
		printf("%02X-", mac[i]);
	}
	printf("%02X", mac[5]);
}

void printIp(DWORD ip){
	in_addr addr;
	addr.s_addr = ip;
	printf("%s", inet_ntoa(addr));
}

void printAlldevs() {
	int index = 0;
	cout << "�����������Ϣ" << endl;
	// ��ȡ���ػ����豸�б�
	if (pcap_findalldevs(&alldevs, errbuf)  != -1){
		/* ��ӡ������Ϣ�б� */
		for (d = alldevs; d != NULL; d = d->next)
		{
			++index;
			if (d->description) {
				printf("%s ID: %d --> Name: %s \n", d->name, index, d->description);
			}
			printIf(d);
		}
	}
	pcap_freealldevs(alldevs);
}

void printIf(pcap_if_t* d) {
	pcap_addr_t* a;
	for (a = d->addresses; a; a = a->next)
	{
		if (a->addr->sa_family == AF_INET) {
			printf("  Address Family: #%d\n", a->addr->sa_family);
			printf("  Address Family Name: AF_INET\n");
			if (a->addr) {
				printf("  IP Address: %s\n", inet_ntoa(((struct sockaddr_in*)a->addr)->sin_addr));
			}
			if (a->netmask) {
				printf("  Netmask: %s\n", inet_ntoa(((struct sockaddr_in*)a->netmask)->sin_addr));
			}
			if (a->broadaddr) {
				printf("  Broadcast Address: %s\n", inet_ntoa(((sockaddr_in*)a->broadaddr)->sin_addr));
			}
			if (a->dstaddr) {
				printf("  Destination Address: %s\n", inet_ntoa(((struct sockaddr_in*)a->dstaddr)->sin_addr));
			}
		}
	}
	printf("\n");
}

pcap_t* open(int choose) {
	if (pcap_findalldevs(&alldevs, errbuf) != -1) {
		for (int x = 0; x < choose - 1; ++x) { 
			alldevs = alldevs->next;
		}
	}
	int index = 0;
	for (a = alldevs->addresses; a != nullptr; a = a->next) {
		if (a->addr->sa_family == AF_INET) { //�жϸõ�ַ�Ƿ�IP��ַ
			ip[index] = ((struct sockaddr_in*)a->addr)->sin_addr.s_addr;
			mask[index] = ((struct sockaddr_in*)a->netmask)->sin_addr.s_addr;
			index++;
		}
	}
	pcap_t* adhandle = pcap_open(alldevs->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 100, NULL, errbuf);
	return adhandle;
}

DWORD getNet(DWORD ip, DWORD mask) {
	return ip & mask;
}

void getSelfMac(pcap_t* adhandle, DWORD ip, BYTE* mac) {
	ARP_t ARPFrame;
	ARPFrame.FrameHeader = ETH_HRD_DEFAULT;//��̫��֡ͷ
	ARPFrame.ARPHeader = ARP_HRD_DEFAULT;//ARP֡ͷ
	ARPFrame.ARPHeader.SendIP = inet_addr("100.100.100.100");
	ARPFrame.ARPHeader.RecvIP = ip;
	if (pcap_sendpacket(adhandle, (const u_char*)&ARPFrame, 42) != 0) {
		printf("error\n");
		return;
	}
	int res;
	while ((res = pcap_next_ex(adhandle, &pkt_header, &pkt_data)) >= 0) {
		if (res == 0)continue;
		if (*(WORD*)(pkt_data + 12) == htons(EH_TYPE)
			&& *(WORD*)(pkt_data + 20) == htons(ARP_REPLY)
			&& *(DWORD*)(pkt_data + 38)
			== inet_addr("100.100.100.100")) {
			for (int i = 0; i < 6; i++) {
				mac[i] = *(BYTE*)(pkt_data + 22 + i);
			}
			printf("��ȡ����MAC��ַ�ɹ�!\n");
			break;
		}
	}
}

void getOtherMac(pcap_t* adhandle, DWORD ip_, BYTE* mac_){
	memset(mac_, 0, sizeof(mac_));
	ARP_t ARPFrame;
	for (int i = 0; i < 6; i++) {
		ARPFrame.FrameHeader.DesMAC[i] = 0xff;
		ARPFrame.FrameHeader.SrcMAC[i] = mac[i];
		ARPFrame.ARPHeader.SendHa[i] = mac[i];
		ARPFrame.ARPHeader.RecvHa[i] = 0x0f;
	}
	ARPFrame.FrameHeader.FrameType = htons(0x0806);							//֡����ΪARP
	ARPFrame.ARPHeader.HardwareType = htons(0x0001);						//Ӳ������Ϊ��̫��
	ARPFrame.ARPHeader.ProtocolType = htons(0x0800);						//Э������ΪIP
	ARPFrame.ARPHeader.HLen = 6;											//Ӳ����ַ����Ϊ6
	ARPFrame.ARPHeader.PLen = 4;											//Э���ַ��Ϊ4
	ARPFrame.ARPHeader.Operation = htons(0x0001);							//����ΪARP����
	ARPFrame.ARPHeader.SendIP = ip[0];										//���ͷ�:����IP
	ARPFrame.ARPHeader.RecvIP = ip_;										//���շ�:����mac��ַ��ip

	if (pcap_sendpacket(adhandle, (u_char*)&ARPFrame, sizeof(ARPFrame)) != 0) {
		//���ʹ�����
		printf("senderror\n");
	}
	else{
		while (1){
			pcap_pkthdr* pkt_header;
			const u_char* pkt_data;
			int rtn = pcap_next_ex(adhandle, &pkt_header, &pkt_data);
			if (rtn == 1){
				ARP_t* arpPacket = (ARP_t*)pkt_data;
				if (ntohs(arpPacket->FrameHeader.FrameType) == 0x0806){
					if (compare(arpPacket->FrameHeader.DesMAC, ARPFrame.FrameHeader.SrcMAC) && arpPacket->ARPHeader.SendIP == ip_){
						for (int i = 0; i < 6; i++){
							mac_[i] = arpPacket->FrameHeader.SrcMAC[i];
						}
						if (arpTable::lookup(ip_, mac) == -1) {
							arpTable::insert(ip_, mac_);
						}
						break;
					}
				}
			}
		}
	}
}

void sendARP(pcap_t* adhandle, DWORD ip_) {
	ARP_t ARPFrame;
	for (int i = 0; i < 6; i++) {
		ARPFrame.FrameHeader.DesMAC[i] = 0xff;
		ARPFrame.FrameHeader.SrcMAC[i] = mac[i];
		ARPFrame.ARPHeader.SendHa[i] = mac[i];
		ARPFrame.ARPHeader.RecvHa[i] = 0x0f;
	}
	ARPFrame.FrameHeader.FrameType = htons(0x0806);							//֡����ΪARP
	ARPFrame.ARPHeader.HardwareType = htons(0x0001);						//Ӳ������Ϊ��̫��
	ARPFrame.ARPHeader.ProtocolType = htons(0x0800);						//Э������ΪIP
	ARPFrame.ARPHeader.HLen = 6;											//Ӳ����ַ����Ϊ6
	ARPFrame.ARPHeader.PLen = 4;											//Э���ַ��Ϊ4
	ARPFrame.ARPHeader.Operation = htons(0x0001);							//����ΪARP����
	ARPFrame.ARPHeader.SendIP = ip[0];										//���ͷ�:����IP
	ARPFrame.ARPHeader.RecvIP = ip_;										//���շ�:����mac��ַ��ip

	if (pcap_sendpacket(adhandle, (u_char*)&ARPFrame, sizeof(ARPFrame)) != 0) {
		//���ʹ�����
		printf("senderror\n");
	}
}

void resend(pcap_t* adhandle, ICMP_t data, DWORD nextip, BYTE dstMac[]) {
	IP_t* temp = (IP_t*)&data;
	if (--(temp->IPHeader.TTL) == 0) {
		//ICMP��ʱ���ķ���......
		//ICMPPacketProc(11, 0, (u_char*)(&data));
		return;
	}
	memcpy(temp->FrameHeader.SrcMAC, temp->FrameHeader.DesMAC, 6);
	memcpy(temp->FrameHeader.DesMAC, dstMac, 6);
	setCheckSum(temp);
	int rtn = pcap_sendpacket(adhandle, (const u_char*)temp, sizeof(data));
	if (rtn == 0) {
		printf("[ת��]\tԴIP��");
		printIp(temp->IPHeader.SrcIP);
		printf("\tĿ��IP��");
		printIp(temp->IPHeader.DstIP);
		printf("\t��һ����");
		if (nextip == 0) {
			printf("ֱ��Ͷ��");
		}
		else {
			printIp(nextip);
		}
		printf("\tTTL��%d\n", temp->IPHeader.TTL);
	}
}

DWORD WINAPI handlerRequest(LPVOID lparam)
{
	routeTable* table = (routeTable*)(LPVOID)lparam;
	pcap_pkthdr* pkt_header;
	const u_char* pkt_data;
	while (1) {
		while (1) {
			int rtn = pcap_next_ex(adhandle, &pkt_header, &pkt_data);
			if (rtn)break;
		}
		IP_t* data = (IP_t*)pkt_data;
		if (compare(data->FrameHeader.DesMAC, mac) && ntohs(data->FrameHeader.FrameType) == 0x0800) {
			printf("[����]\tԴIP��");
			printIp(data->IPHeader.SrcIP);
			printf("\tĿ��IP��");
			printIp(data->IPHeader.DstIP);
			printf("\tTTL��%d\n", data->IPHeader.TTL);
			DWORD dstip = data->IPHeader.DstIP;
			DWORD nextip = table->lookup(dstip);										//����·�ɱ����Ƿ��ж�Ӧ����
			if (nextip == -1)continue;													//û����ֱ�Ӷ���
			if (!CheckSum(data))continue;												//У��Ͳ���ȷֱ�Ӷ���
			if (data->IPHeader.DstIP != ip[0] && data->IPHeader.DstIP != ip[1]) {		//�����߲����Լ�
				BYTE broadmac[6] = { 0xff,0xff,0xff,0xff,0xff,0xff };
				int t1 = compare(data->FrameHeader.DesMAC, broadmac);
				int t2 = compare(data->FrameHeader.SrcMAC, broadmac);
				if (!t1 && !t2) {														//���ǹ㲥��Ϣ
					ICMP_t temp = *(ICMP_t*)pkt_data;
					BYTE dstmac[6];
					if (nextip == 0)													//ֱ��Ͷ�ݣ�����Ŀ��IP��MAC
					{
						if (arpTable::lookup(temp.IPHeader.DstIP, dstmac) == -1) {
							sendARP(adhandle, temp.IPHeader.DstIP);
							cache(buffer, BufferSize, &temp, temp.IPHeader.DstIP);
							continue;
						}
							resend(adhandle, temp, nextip, dstmac);
					}
					else {																//��ֱ��Ͷ�ݣ�������һ��IP��MAC
						if (arpTable::lookup(nextip, dstmac) == -1) {
							sendARP(adhandle, nextip);
							cache(buffer, BufferSize, &temp, nextip);
							continue;
						}
							resend(adhandle, temp, nextip, dstmac);
					}
				}
			}
		}
		else if (compare(data->FrameHeader.DesMAC, mac) && ntohs(data->FrameHeader.FrameType) == 0x0806) {
			ARP_t* arpPacket = (ARP_t*)pkt_data;
			if (ntohs(arpPacket->ARPHeader.Operation) == ARP_REPLY) {
				DWORD ip_ = arpPacket->ARPHeader.SendIP;
				BYTE mac_[6] = { 0 };
				memcpy(mac_, arpPacket->ARPHeader.SendHa, 6);
				if (arpTable::lookup(ip_, mac_) == -1) {
					arpTable::insert(ip_, mac_);
					printf("[ARP]\tIP��");
					printIp(ip_);
					printf("\tMAC��");
					printMac(mac_);
					printf("\n");
				}
			}
		}
	}
}

int find(Buffer* buffer, int size) {
	for (int i = 0; i < size; i++) {
		if (buffer[i].valid) {
			return i;
		}
	}
	return -1;
}

void handle(Buffer* buffer, int size) {
	for (int i = 0; i < size; i++) {
		if (buffer[i].valid == 0) { continue; }
		if ((clock() - buffer[i].clock) > BufferTime) {
			buffer[i].valid = 0;
			continue;
		}
		BYTE mac_[6];
		if (arpTable::lookup(buffer[i].TargetIP, mac_) != -1) {
			ICMP_t tmp;
			memcpy(&tmp, 0, sizeof(tmp));
			memcpy(&tmp, &buffer[i].Data, sizeof(tmp));
			resend(adhandle, tmp, buffer[i].TargetIP, mac_);
			buffer[i].valid = 0;
		}
	}
}

void cache(Buffer* buffer, int size, ICMP_t* data, DWORD TargetIP) {
	int index = find(buffer, BufferSize);
	if (index == -1) {
		cout << "����������������" << endl;
		return;
	}
	memset(&buffer[index].Data, 0, DataSize);
	memcpy(&buffer[index].Data, data, sizeof(ICMP_t));
	buffer[index].TargetIP = data->IPHeader.DstIP;
	buffer[index].clock = clock();
	buffer[index].valid = 1;
}

void ICMPPacket(BYTE type, BYTE code, const u_char* pkt_data) {
	ICMP_t packet;
	//����FrameHeader_t
	memcpy(packet.FrameHeader.DesMAC, ((FrameHeader_t*)pkt_data)->SrcMAC, 6);
	memcpy(packet.FrameHeader.SrcMAC, ((FrameHeader_t*)pkt_data)->DesMAC, 6);
	packet.FrameHeader.FrameType = htons(0x0800);
	//����IPHeader_t
	packet.IPHeader.Ver_HLen = ((IPHeader_t*)(pkt_data + 14))->Ver_HLen;
	packet.IPHeader.TOS = ((IPHeader_t*)(pkt_data + 14))->TOS;
	packet.IPHeader.TotalLen = htons(56);
	packet.IPHeader.ID = ((IPHeader_t*)(pkt_data + 14))->ID;
	packet.IPHeader.Flag_Segment = ((IPHeader_t*)(pkt_data + 14))->Flag_Segment;
	packet.IPHeader.TTL = 64;
	packet.IPHeader.Protocol = 1;
	packet.IPHeader.SrcIP = ip[0];
	packet.IPHeader.DstIP = ((IPHeader_t*)(pkt_data + 14))->SrcIP;
	setCheckSum((IP_t*)(&packet));
	//����ICMPHeader_t
	packet.ICMPHeader.Type = type;
	packet.ICMPHeader.Code = code;
	packet.ICMPHeader.Id = 0;
	packet.ICMPHeader.Sequence = 0;
	packet.ICMPHeader.Checksum = 0;
	memcpy(((u_char*)(&packet) + 42), (IPHeader_t*)(pkt_data + 14), 20);
	memcpy(((u_char*)(&packet) + 62), (u_char*)(pkt_data + 34), 8);
	unsigned int sum = 0;
	WORD* tmp = (WORD*)&(packet.ICMPHeader);
	for (int i = 0; i < 5; i++) {
		sum += tmp[i];
		if (sum & 0xffff0000) {
			sum &= 0xffff;
			sum++;
		}
	}
	packet.ICMPHeader.Checksum = ~sum;
	cout << ~sum << endl;
	pcap_sendpacket(adhandle, (u_char*)&packet, 70);
}

