#include "Server.h"
u_char Seq = 0;//当前要接收的序列号
WSADATA wsaData;
SOCKET Server;
SOCKADDR_IN ServerAddr, ClientAddr;
clock_t beg;
string FileName;
char FileBuf[200000000];
int FileSize;
char sendBuf[2000000];
char recvBuf[2000000]; 
void init() {
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw("WSADATA init false! error code : %d", WSAGetLastError());
	}
	printf("WSADATA init success!\n");

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	Server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Server == INVALID_SOCKET) {
		closesocket(Server);
		throw("socket of server invalid!");
	}
	printf("create socket of server success!\n");

	if (bind(Server, (sockaddr*)(&ServerAddr), sizeof(ServerAddr)) == SOCKET_ERROR) {
		closesocket(Server);
		WSACleanup();
		throw("bind fail!");
	}
	printf("bind success!\n");

	int recvbuf = 1;
	setsockopt(Server, SOL_SOCKET, SO_RCVBUF, (char*)&recvbuf, sizeof(int));
	//设置为非阻塞模式  
	int imode = 1;
	ioctlsocket(Server, FIONBIO, (u_long*)&imode);
}
void release() {
	closesocket(Server);
	WSACleanup();
	printf("program will close after 3 seconds\n");
	Sleep(3000);
}
bool ShakeHand() {
	Head recv;
	int len = sizeof(ClientAddr);
	while (true) {
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			if (CheckSum((u_short*)&recv, sizeof(recv)) == 0 && (recv.type == SYN)) {
				Seq++;
				break;
			}
		}
	}
	Head h2;
	h2.seq = Seq;
	h2.type = SYN | ACK;
	h2.checksum = 0;
	h2.checksum = CheckSum((u_short*)&h2, sizeof(h2));
	if (sendto(Server, (char*)&h2, sizeof(h2), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	while (true) {
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			if (CheckSum((u_short*)&recv, sizeof(recv)) == 0 && (recv.type == ACK) && recv.seq == Seq) {
				Seq += 1;
				return true;
			}
			else if (recv.seq < Seq){
				continue;
			}
			else {
				printf("客户端建立连接失败\n");
				return false;
			}
		}
	}
}

string RecvFileName() {
	string filename;
	Head head;
	char recv[2000];
	memset(recv, 0, sizeof(recv));
	int len = sizeof(ClientAddr);
	while (true) {
		if (recvfrom(Server, recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			memcpy((char*)&head, recv, sizeof(head));
			if (CheckSum((u_short*)&recv, sizeof(head)+head.length) == 0 && (head.type == FILE) && head.seq == Seq) {
				filename = recv + sizeof(head);
				Seq += 1;
				break;
			}
		}
	}
	Head reply;
	reply.seq = Seq;
	reply.type = ACK;
	reply.checksum = 0;
	reply.checksum = CheckSum((u_short*)&reply, sizeof(reply));
	if (sendto(Server, (char*)&reply, sizeof(reply), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		cout << ("Error: fail to send messages!");
	}
	return filename;
}

void recvData() {
	srand((int)time(0));
	int SendNum = 0;
	Head head;
	int len = sizeof(ClientAddr);
	while (true) {
		bool end = 0;
		memset(recvBuf, 0, sizeof(recvBuf));
		while (true) {
			if (recvfrom(Server, recvBuf, sizeof(recvBuf), 0, (sockaddr*)&ClientAddr, &len) > 0) {
				if (lossInLossRatio(lossRatio)) { continue; }
				memcpy((char*)&head, recvBuf, sizeof(head));
				if (CheckSum((u_short*)&recvBuf, sizeof(head) + head.length) == 0 && (head.type & SEND) && head.seq == Seq) {
					if (head.type & LAST) {
						end = 1;
					}
					memcpy(FileBuf + MAX_SEND_SIZE * SendNum, recvBuf + sizeof(head), head.length);
					cout << "Server: " << "[ACK]" << "ack = " << int(Seq) << endl;
					FileSize += head.length;
					SendNum++;
					Seq += 1;

					Head reply;
					reply.seq = Seq;
					reply.type = ACK;
					if (Seq == 50) {
						reply.type = -1;
					}
					reply.checksum = 0;
					reply.checksum = CheckSum((u_short*)&reply, sizeof(reply));
					if (sendto(Server, (char*)&reply, sizeof(reply), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
						cout << ("Error: fail to send messages!");
					}
					break;
				}
				else {
					Head reply;
					reply.seq = Seq;
					reply.type = ACK;
					reply.checksum = 0;
					reply.checksum = CheckSum((u_short*)&reply, sizeof(reply));
					if (sendto(Server, (char*)&reply, sizeof(reply), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
						cout << ("Error: fail to send messages!");
					}
					break;
				}
			}
		}
		if (end == 1) {
			cout << "接收完毕" << endl;
			break;
		}
	}

}

bool WaveHand() {
	Head recv;
	int len = sizeof(ClientAddr);
	while (true) {
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			if (CheckSum((u_short*)&recv, sizeof(recv)) == 0 && (recv.type == (FIN | ACK))) {
				cout << "Client [FIN, ACK]" << "Seq = " << int(recv.seq) << endl;
				Seq = recv.seq + 1;
				break;
			}
		}
	}
	Head h2;
	h2.seq = Seq;
	h2.type = ACK;
	h2.checksum = 0;
	h2.checksum = CheckSum((u_short*)&h2, sizeof(h2));
	if (sendto(Server, (char*)&h2, sizeof(h2), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	cout << "Server [ACK]" << "ack = " << int(Seq) << endl;
	Seq++;
	return true;
}

u_short CheckSum(u_short* buf, int length) {
	int count = (length + 1) / 2;
	u_int sum = 0;
	while (count--) {
		sum += *buf++;
		if (sum & 0xffff0000) {
			sum &= 0xffff;
			sum++;
		}
	}
	return ~(sum & 0xffff);
}
int random() {
	srand((int)time(0));
	return rand();
}

BOOL lossInLossRatio(float lossRatio) {
	int lossBound = (int)(lossRatio * 100);
	int r = rand() % 101;
	if (r <= lossBound) {
		return TRUE;
	}
	return FALSE;
}
