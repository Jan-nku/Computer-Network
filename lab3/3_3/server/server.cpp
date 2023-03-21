#include"server.h"
//Socket通讯
WSADATA wsaData;
SOCKET Server;
SOCKADDR_IN ServerAddr, ClientAddr;

//序列号
u_char Seq;

//接收数据
char FileName[20];			//文件名
u_int FileSize;				//文件大小
char Buffer[100000000];		//文件缓冲区
char recvBuf[20000];		//数据包接收缓冲区

//函数
//初始化、释放
void init() {
	//WSAStartup
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw("WSADATA init false! error code : %d", WSAGetLastError());
	}
	printf("WSADATA init success!\n");

	//ServerAddr初始化
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

	//绑定Server、ServerAddr
	if (bind(Server, (sockaddr*)(&ServerAddr), sizeof(ServerAddr)) == SOCKET_ERROR) {
		closesocket(Server);
		WSACleanup();
		throw("bind fail!");
	}
	printf("bind success!\n");

	//设置为非阻塞模式  
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
//握手挥手
bool ShakeHand() {
	//接收数据包recv
	Head recv;
	int len = sizeof(ServerAddr);
	while (1) {
		//接收到数据包，首先清空缓冲区
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			//验证消息类型、校验和、序列号
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && (recv.type == SYN)) {
				//接收到正确的数据包
				//打印日志
				cout << "[SYN]Client:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum = " << recv.checksum << endl;
				//序列号增1
				Seq++;
				break;
			}
		}
	}

	//封装数据包send
	Head send;
	send.seq = Seq;
	send.type = SYN | ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));
	
	//发送数据包
	if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}

	//打印日志
	cout << "[SYN, ACK]Server:\t\t" << "Seq = " << int(send.seq) << "\tCheckSum = " << send.checksum << endl;
	
	//启动计时器
	clock_t Begin = clock();

	while (1) {
		//接收数据包，首先清除缓冲区
		memset(&recv, 0, sizeof(recv));
		//接收到数据包
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			//验证消息类型、校验和、序列号
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && (recv.type == ACK) && recv.seq == Seq) {
				//序列号增1
				Seq++;
				//打印日志并退出
				cout << "[ACK]Client:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum = " << recv.checksum << endl;
				return true;
			}
			//接收到之前的数据包，什么也不做
			else if (recv.seq < Seq) {
				continue;
			}
			else {
				//数据包出现差错，需要重传
				cout << "第" << send.seq << "号数据包差错重传..." << endl;
				if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				//重新计时
				Begin = clock();
			}
		}
		else if (clock() - Begin > MAX_WAIT_TIME) {
			//计时器超时，需要重传
			cout << "第" << send.seq << "号数据包超时重传..." << endl;
			if (sendto(Server, (char*)&send, sizeof(send), 0, (sockaddr*)&ClientAddr, sizeof(ClientAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			//重新计时
			Begin = clock();
		}
	}
}
bool WaveHand() {
	//接收数据包recv
	Head recv;
	int len = sizeof(ServerAddr);
	while (1) {
		//接收数据包，首先清除缓冲区
		memset(&recv, 0, sizeof(recv));
		//接收到数据包
		if (recvfrom(Server, (char*)&recv, sizeof(recv), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			//验证消息类型、校验和、序列号
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && (recv.type == (FIN | ACK))) {
				//打印日志
				cout << "[FIN, SYN]Client:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum = " << recv.checksum << endl;
				//序列号增1
				Seq++;
				break;
			}
		}
	}


	//封装数据包send
	Head send;
	send.seq = Seq;
	send.type = ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));
	//发送数据包
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
	//打印日志
	cout << "[ACK]Server:\t\t" << "Seq = " << int(send.seq) << "\tCheckSum = " << send.checksum << endl;
	//序列号增1
	Seq++;
	return true;
}

//接收数据
void recvData()
{
	//清空文件缓冲区
	FileSize = 0;
	memset(Buffer, 0, sizeof(Buffer));

	//recv、send head
	Head recv;
	Head send;
	int len = sizeof(ClientAddr);

	while (1){
		//接收数据包，首先要清空recvBuf
		memset(recvBuf, 0, sizeof(recvBuf));
		//接收到数据包
		if (recvfrom(Server, (char*)&recvBuf, sizeof(recvBuf), 0, (sockaddr*)&ClientAddr, &len) > 0) {
			Padding(recv, recvBuf);
			//如果是over，说明是最后一个数据包

			if (recv.type == OVER && !CheckSum((u_short*)recvBuf, sizeof(recv) + recv.length)) {
				cout << "[SEND]Client: " << "Seq = " << int(recv.seq) << "  Window = " << int(recv.window)
					<< "  Length = " << recv.length << "  CheckSum = " << int(recv.checksum) << endl;
				//序列号增1
				Seq++;
				//缓存数据包data
				memcpy(Buffer + FileSize, recvBuf + sizeof(Head), recv.length);
				FileSize += recv.length;
				break;
			}
			//接收数据包
			if (recv.type == SEND && !CheckSum((u_short*)recvBuf, sizeof(recv) + recv.length)) {
				//接收到当前需要的数据包，处理
				if (Seq == recv.seq) {
					cout << "[SEND]Client: " << "Seq = " << int(recv.seq) << "  Window = " << int(recv.window)
						<< "  Length = " << recv.length << "  CheckSum = " << int(recv.checksum) << endl;

					//序列号增1
					Seq++;
					//缓存数据包data
					memcpy(Buffer + FileSize, recvBuf + sizeof(Head), recv.length);
					FileSize += recv.length;
				}
				//返回当前要接收分组的Seq
				//如果Seq < recv.seq，数据包不会处理，但同样返回ACK
				Packet(send, ACK, Seq);
				//发送数据包
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

//计算数据包校验和并返回
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

//数据包相关函数
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

