#include"server.h"
//SocketͨѶ
WSADATA wsaData;
SOCKET Server;
SOCKADDR_IN ServerAddr, ClientAddr;

//���к�
u_char Seq;

//��������
char FileName[20];			//�ļ���
u_int FileSize;				//�ļ���С
char Buffer[100000000];		//�ļ�������
char recvBuf[20000];		//���ݰ����ջ�����

//����
//��ʼ�����ͷ�
void init() {
	//WSAStartup
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw("WSADATA init false! error code : %d", WSAGetLastError());
	}
	printf("WSADATA init success!\n");

	//ServerAddr��ʼ��
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	//socket Server
	Server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Server == INVALID_SOCKET) {
		closesocket(Server);
		throw("socket of server invalid!");
	}
	printf("create socket of server success!\n");

	//��Server��ServerAddr
	if (bind(Server, (sockaddr*)(&ServerAddr), sizeof(ServerAddr)) == SOCKET_ERROR) {
		closesocket(Server);
		WSACleanup();
		throw("bind fail!");
	}
	printf("bind success!\n");

	//����Ϊ������ģʽ  
	int recvbuf = 1;
	setsockopt(Server, SOL_SOCKET, SO_RCVBUF, (char*)&recvbuf, sizeof(int));
	int imode = 1;
	ioctlsocket(Server, FIONBIO, (u_long*)&imode);
}
void release() {
	//closesocket
	closesocket(Server);

	//WSACleanup
	WSACleanup();
	printf("program will close after 3 seconds\n");
	Sleep(3000);
}
//���ֻ���
bool ShakeHand() {
	//�������ݰ�recv
	Head recv;
	int len = sizeof(ServerAddr);
	while (1) {
		//���յ����ݰ���������ջ�����
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			//��֤��Ϣ���͡�У��͡����к�
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && (recv.type == SYN)) {
				//���յ���ȷ�����ݰ�
				//��ӡ��־
				cout << "[SYN]Client:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum = " << recv.checksum << endl;
				//���к���1
				Seq++;
				break;
			}
		}
	}

	//��װ���ݰ�send
	Head send;
	send.seq = Seq;
	send.type = SYN | ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));
	
	//�������ݰ�
	if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}

	//��ӡ��־
	cout << "[SYN, ACK]Server:\t\t" << "Seq = " << int(send.seq) << "\tCheckSum = " << send.checksum << endl;
	
	//������ʱ��
	clock_t Begin = clock();

	while (1) {
		//�������ݰ����������������
		memset(&recv, 0, sizeof(recv));
		//���յ����ݰ�
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			//��֤��Ϣ���͡�У��͡����к�
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && (recv.type == ACK) && recv.seq == Seq) {
				//���к���1
				Seq++;
				//��ӡ��־���˳�
				cout << "[ACK]Client:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum = " << recv.checksum << endl;
				return true;
			}
			//���յ�֮ǰ�����ݰ���ʲôҲ����
			else if (recv.seq < Seq) {
				continue;
			}
			else {
				//���ݰ����ֲ����Ҫ�ش�
				cout << "��" << send.seq << "�����ݰ�����ش�..." << endl;
				if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				//���¼�ʱ
				Begin = clock();
			}
		}
		else if (clock() - Begin > MAX_WAIT_TIME) {
			//��ʱ����ʱ����Ҫ�ش�
			cout << "��" << send.seq << "�����ݰ���ʱ�ش�..." << endl;
			if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			//���¼�ʱ
			Begin = clock();
		}
	}
}
bool WaveHand() {
	//�������ݰ�recv
	Head recv;
	int len = sizeof(ServerAddr);
	while (1) {
		//�������ݰ����������������
		memset(&recv, 0, sizeof(recv));
		//���յ����ݰ�
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			//��֤��Ϣ���͡�У��͡����к�
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && (recv.type == (FIN | ACK))) {
				//��ӡ��־
				cout << "[FIN, SYN]Client:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum = " << recv.checksum << endl;
				//���к���1
				Seq++;
				break;
			}
		}
	}


	//��װ���ݰ�send
	Head send;
	send.seq = Seq;
	send.type = ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));
	//�������ݰ�
	if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	//��ӡ��־
	cout << "[ACK]Server:\t\t" << "Seq = " << int(send.seq) << "\tCheckSum = " << send.checksum << endl;
	//���к���1
	Seq++;
	return true;
}

//��������
void recvData()
{
	//����ļ�������
	FileSize = 0;
	memset(Buffer, 0, sizeof(Buffer));

	//recv��send head
	Head recv;
	Head send;
	int len = sizeof(ClientAddr);

	while (1){
		//�������ݰ�������Ҫ���recvBuf
		memset(recvBuf, 0, sizeof(recvBuf));
		//���յ����ݰ�
		if (recvfrom(Server, (char*)&recvBuf, sizeof(recvBuf), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			Padding(recv, recvBuf);
			//�����over��˵�������һ�����ݰ�

			if (recv.type == OVER && !CheckSum((u_short*)recvBuf, sizeof(recv) + recv.length)) {
				cout << "[SEND]Client: " << "Seq = " << int(recv.seq) << "  Window = " << int(recv.window)
					<< "  Length = " << recv.length << "  CheckSum = " << int(recv.checksum) << endl;
				//���к���1
				Seq++;
				//�������ݰ�data
				memcpy(Buffer + FileSize, recvBuf + sizeof(Head), recv.length);
				FileSize += recv.length;
				break;
			}
			//�������ݰ�
			if (recv.type == SEND && !CheckSum((u_short*)recvBuf, sizeof(recv) + recv.length)) {
				//���յ���ǰ��Ҫ�����ݰ�������
				if (Seq == recv.seq) {
					cout << "[SEND]Client: " << "Seq = " << int(recv.seq) << "  Window = " << int(recv.window)
						<< "  Length = " << recv.length << "  CheckSum = " << int(recv.checksum) << endl;

					//���к���1
					Seq++;
					//�������ݰ�data
					memcpy(Buffer + FileSize, recvBuf + sizeof(Head), recv.length);
					FileSize += recv.length;
				}
				//���ص�ǰҪ���շ����Seq
				//���Seq < recv.seq�����ݰ����ᴦ����ͬ������ACK
				Packet(send, ACK, Seq);
				//�������ݰ�
				if (sendto(Server, (char*)&send, sizeof(Head), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
					throw("Error: fail to send messages!");
				}
			}
		}
	}
	Packet(send, ACK, Seq);
	if (sendto(Server, (char*)&send, sizeof(Head), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
	}
	cout << "[ACK]" << "Server: " << "Seq = " << int(send.seq) << "  Window = " << int(send.window)
		<< "  Length = " << send.length << "  CheckSum = " << int(send.checksum) << endl;
}

//�������ݰ�У��Ͳ�����
u_short CheckSum(u_short* buffer, int length) {
	int count = (length + 1) / 2;
	u_int sum = 0;
	while (count--) {
		sum += *buffer++;
		if (sum & 0xffff0000) {
			sum &= 0xffff;
			sum++;
		}
	}
	return ~(sum & 0xffff);
}

//���ݰ���غ���
//Clear head
void Clear(Head& head) {
	memset(&head, 0, sizeof(head));
}
//Padding head
void Padding(Head& head, char* recvBuf) {
	memset(&head, 0, sizeof(head));
	memcpy(&head, recvBuf, sizeof(head));
}
//Packet head
void Packet(Head& head, u_char type, u_char seq) {
	memset(&head, 0, sizeof(head));
	head.type = type;
	head.seq = seq;
	head.checksum = CheckSum((u_short*)&head, sizeof(head));
}

