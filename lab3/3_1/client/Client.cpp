#include "Client.h"

WSADATA wsaData;
SOCKET Client;
SOCKADDR_IN ServerAddr;
u_char Seq = 0;
clock_t Begin;
string FileName = "1.jpg";
char FileBuf[200000000];
int FileSize;
char sendBuf[2000000];
char recvBuf[2000000];
int SendNum;
int TotalNum;
clock_t SendTime;

void init() {
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw("WSADATA init false! error code : %d", WSAGetLastError());
	}
	printf("WSADATA init success!\n");

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.S_un.S_addr = inet_addr(IP);
	Client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Client == INVALID_SOCKET) {
		closesocket(Client);
		throw("socket of client invalid!");
	}
	printf("create socket of client success!\n");
	int recvbuf = 1;
	setsockopt(Client, SOL_SOCKET, SO_RCVBUF, (char*)&recvbuf, sizeof(int));
	//设置为非阻塞模式  
	int imode = 1;
	ioctlsocket(Client, FIONBIO, (u_long*)&imode);
}
void SendFileName(string FileName) {
	cout << "传送文件名ing..." << endl;
	Head head;
	head.seq = Seq;
	head.length = FileName.length();
	head.type = FILE;
	head.checksum = 0;
	memset(sendBuf, 0, sizeof(sendBuf));
	memcpy(sendBuf, &head, sizeof(head));
	memcpy(sendBuf + sizeof(head), FileName.c_str(), head.length);
	head.checksum = CheckSum((u_short*)sendBuf, sizeof(head) + head.length);
	memcpy(sendBuf, &head, sizeof(head));
	if (sendto(Client, sendBuf, head.length + sizeof(head), 0, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR)) == -1) {
		cout << "Error: fail to send messages!" << endl;
	}
	cout << "Client: " << "[FILE]" << "seq=" << int(Seq) << endl;
	Seq++;
	Begin = clock();
	memset(recvBuf, 0, sizeof(recvBuf));
	int len = sizeof(ServerAddr);
	while (true) {
		if (recvfrom(Client, (char*)&recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			Head recv;
			memcpy((char*)&recv, recvBuf, sizeof(recv));
			if (CheckSum((u_short*)&recvBuf, sizeof(recvBuf)) == 0 && recv.type == ACK && recv.seq == Seq) {
				//验证消息类型、校验和、序列号
				break;
			}
			else {
				//差错重传
				if (sendto(Client, sendBuf, head.length + sizeof(head), 0, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR)) == -1) {
					throw("Error: fail to send messages!");
				}
				Begin = clock();
			}
		}
		else if (clock() - Begin > MAX_WAIT_TIME) {
			//超时重传
			if (sendto(Client, sendBuf, head.length + sizeof(head), 0, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR)) == -1) {
				throw("Error: fail to send messages!");
			}
			Begin = clock();
		}
	}
}

void Send_Message(char* data, int length) {
	SendTime = clock_t();
	TotalNum = length / MAX_SEND_SIZE + (length % MAX_SEND_SIZE != 0);
	cout << "传送数据ing..." << endl;
	SendNum = 0;
	Head head;
	while (true) {
		if (SendNum == TotalNum) {
			break;
		}
		head.seq = Seq;
		head.length = MAX_SEND_SIZE;
		head.type = SEND;
		head.checksum = 0;
		if (SendNum == TotalNum - 1) {
			head.type = SEND | LAST;
			head.length = length % MAX_SEND_SIZE;
		}
		memset(sendBuf, 0, sizeof(sendBuf));
		memcpy(sendBuf, &head, sizeof(head));
		memcpy(sendBuf + sizeof(head), data + SendNum * MAX_SEND_SIZE, head.length);
		head.checksum = CheckSum((u_short*)sendBuf, sizeof(head) + head.length);
		memcpy(sendBuf, &head, sizeof(head));
		if (sendto(Client, sendBuf, head.length + sizeof(head), 0, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR)) == -1) {
			cout << "Error: fail to send messages!" << endl;
		}
		cout << "Client: " << "[Send]" << "seq=" << int(Seq) << "	length:" << head.length << "	checksum:" << head.checksum << endl;;
		SendNum++;
		Seq++;
		Begin = clock();
		int len = sizeof(ServerAddr);
		while (true) {
			memset(recvBuf, 0, sizeof(recvBuf));
			if (recvfrom(Client, (char*)&recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
				Head recv;
				memcpy((char*)&recv, recvBuf, sizeof(recv));
				if (CheckSum((u_short*)&recvBuf, sizeof(recvBuf)) == 0 && recv.type == ACK && recv.seq == Seq) {
					//验证消息类型、校验和、序列号
					break;
				}
				else {
					//差错重传
					if (sendto(Client, sendBuf, head.length + sizeof(head), 0, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR)) == -1) {
						throw("Error: fail to send messages!");
					}
					cout << "出错重传序列号为" << int(Seq) - 1 << "报文" << endl;
					Begin = clock();
				}
			}
			else if (clock() - Begin > MAX_WAIT_TIME) {
				//超时重传
				if (sendto(Client, sendBuf, head.length + sizeof(head), 0, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR)) == -1) {
					throw("Error: fail to send messages!");
				}
				cout << "超时重传序列号为" << int(Seq) - 1 << "报文" << endl;
				Begin = clock();
			}
		}
	}
	SendTime = clock() - SendTime;
}

bool WaveHand() {
	cout << "Wave Hand..." << endl;
	Head h1;
	h1.seq = Seq;
	h1.length = 0;
	h1.type = FIN | ACK;
	h1.checksum = 0;
	h1.checksum = CheckSum((u_short*)&h1, sizeof(Head));
	if (sendto(Client, (char*)&h1, sizeof(h1), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	Seq++;
	cout << "client: " << "[FIN,ACK]" << "seq=" << int(h1.seq) << endl;
	Begin = clock();
	Head recv;
	int len = sizeof(ServerAddr);
	while (true) {
		if (recvfrom(Client, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			if (CheckSum((u_short*)&recv, sizeof(recv)) == 0 && recv.type == (ACK) && recv.seq == Seq) {
				//验证消息类型、校验和、序列号
				cout << "server: " << "[ACK]" << "ack=" << int(recv.seq) << endl;
				break;
			}
			else {
				//差错重传
				if (sendto(Client, (char*)&h1, sizeof(h1), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				Begin = clock();
			}
		}
		else if (clock() - Begin > MAX_WAIT_TIME) {
			//超时重传
			if (sendto(Client, (char*)&h1, sizeof(h1), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			Begin = clock();
		}
	}
	return true;
}

void release() {
	closesocket(Client);
	WSACleanup();
	printf("program will close after 3 seconds\n");
	Sleep(3000);
}
bool ShakeHand() {
	Head h1;
	h1.seq = Seq;
	h1.length = 0;
	h1.type = SYN;
	h1.checksum = 0;
	h1.checksum = CheckSum((u_short*)&h1, sizeof(Head));
	if (sendto(Client, (char*)&h1, sizeof(h1), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	Seq++;
	cout << "Client: " << "[SYN]" << "Seq = " << int(h1.seq) << endl;

	Begin = clock();
	Head recv;
	int len = sizeof(ServerAddr);
	while (true) {
		if (recvfrom(Client, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			if (CheckSum((u_short*)&recv, sizeof(recv)) == 0 && recv.type == (SYN | ACK) && recv.seq == Seq) {
				//验证消息类型、校验和、序列号
				cout << "Server [SYN, ACK]" << "ack = " << int(Seq) << endl;
				break;
			}
			else {
				//差错重传
				if (sendto(Client, (char*)&h1, sizeof(h1), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				Begin = clock();
			}
		}
		else if (clock() - Begin > MAX_WAIT_TIME) {
			//超时重传
			if (sendto(Client, (char*)&h1, sizeof(h1), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			Begin = clock();
		}
	}

	Head h3;
	h3.seq = Seq;
	h3.type = ACK;
	h3.length = 0;
	h3.checksum = 0;
	h3.checksum = CheckSum((u_short*)&h3, sizeof(h3));
	if (sendto(Client, (char*)&h3, sizeof(h3), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	Seq++;
	cout << "client: " << "[ACK]" << "seq=" << int(h3.seq) << endl;
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
void ReadFile() {
	int option;
	cout << "选择要上传的文件：" << endl;
	cout << "[1]1.jpg" << endl;
	cout << "[2]2.jpg" << endl;
	cout << "[3]3.jpg" << endl;
	cout << "[4]helloworld.txt" << endl;
	cin >> option;
	switch (option) {
	case 1:
		FileName = "1.jpg";
		break;
	case 2:
		FileName = "2.jpg";
		break;
	case 3:
		FileName = "3.jpg";
		break;
	case 4:
		FileName = "EULA.txt";
		break;
	default:
		cout << "请输入正确的选项!" << endl;
	}
	ifstream fin(FileName.c_str(), ifstream::in | ios::binary);
	if (!fin) {
		cout << "Error: cannot open file!" << endl;
	}
	fin.seekg(0, fin.end);		
	FileSize = fin.tellg();
	fin.seekg(0, fin.beg);	
	memset(FileBuf, 0, FileSize);
	fin.read(FileBuf, FileSize);
	fin.close();
}


