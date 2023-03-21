#include"client.h"
//定义全局变量
//Socket通讯
WSAData wsaData;
SOCKET Client;
SOCKADDR_IN ServerAddr;

//序列号
u_char Seq;

//文件读写
string FileName;			//文件名
u_int FileSize;				//文件大小
char Buffer[100000000];		//文件缓冲区
char sendBuf[20000];		//发送数据缓冲区

//GBN需要使用的全局变量
int resendCount = 0;		// 重传次数
int base = 2;				// base之前序列号累计确认
bool resend = false;		// 是否超时重传
bool restart = false;		// 是否重新开始计时
bool wait = false;			// 是否因窗口不够而需等待
long long lenCopy = 0;		// 文件数据偏移量的拷贝，供重传时使用

//拥塞控制
const int rwnd = 10;		// 接收通告窗口大小，固定值
double cwnd = 1;			// 拥塞窗口大小
int ssthresh = 8;			// 阈值
unsigned char lastAck = 0;	// 上一个ACK序列号
int dupAck = 0;				// 重复收到的ACK次数
int renoState = 0;			// RENO状态机的状态，0为慢启动，1为拥塞控制，2为快速恢复


//函数
//初始化、释放函数
void init() {
	//WSAStartup
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw("WSADATA init false! error code : %d", WSAGetLastError());
	}
	printf("WSADATA init success!\n");

	//设置ServerAddr
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = inet_addr(IP);
	
	//创建Client Socket
	Client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Client == INVALID_SOCKET) {
		closesocket(Client);
		throw("socket of client invalid!");
	}
	printf("create socket of client success!\n");
	
	//设置为非阻塞模式  
	int recvbuf = 1;
	setsockopt(Client, SOL_SOCKET, SO_RCVBUF, (char*)&recvbuf, sizeof(int));
	int imode = 1;
	ioctlsocket(Client, FIONBIO, (u_long*)&imode);
}
void release() {
	//closesocket
	closesocket(Client);
	//WSAcleanup
	WSACleanup();
	printf("program will close after 3 seconds\n");
	Sleep(3000);
}

//三次握手、两次挥手
bool ShakeHand() {
	//封装数据包send
	Head send;
	send.seq = Seq;
	send.type = SYN;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));

	//发送数据包
	if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}

	//序列号增1
	Seq++;
	//日志打印
	cout << "[SYN]Client:\t\t" << "Seq=" << int(send.seq) << "\tCheckSum=" << send.checksum << endl;

	//开启计时器，检测超时
	clock_t Begin = clock();

	//接收数据包recv
	Head recv;
	int len = sizeof(ServerAddr);
	while (true) {
		//接收数据包，首先清除缓冲区
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(Client, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			//验证消息类型、校验和、序列号
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && recv.type == (SYN | ACK) && recv.seq == Seq) {
				//接收到想要的数据包
				//打印日志并退出
				cout << "[SYN, ACK]Server:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum=" << send.checksum << endl;
				break;
			}
			else {
				//接收到的数据包有错误，需要重传
				cout << "第" << send.seq << "号数据包差错重传..." << endl;
				if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				//重新计时
				Begin = clock();
			}
		}
		//计时器超时
		else if (clock() - Begin > MAX_WAIT_TIME) {
			//重传数据包
			cout << "第" << int(send.seq) << "号数据包超时重传..." << endl;
			if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			//重新计时
			Begin = clock();
		}
	}

	//接收到握手数据包，退出循环
	//封装数据包
	send.seq = Seq;
	send.type = ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(send));

	//发送数据包
	if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	//序列号增1
	Seq++;
	//日志打印
	cout << "[ACK]Client:\t\t" << "Seq=" << int(send.seq) << "\tCheckSum=" << send.checksum << endl;
	return true;
}
bool WaveHand() {
	//封装数据包
	Head send;
	send.seq = Seq;
	send.type = FIN | ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));
	
	//发送数据包
	if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	
	//序列号增1
	Seq++;
	//打印日志
	cout << "[FIN,ACK]Client:\t\t" << "Seq=" << int(send.seq) << "\tCheckSum=" << send.checksum << endl;

	//开启计时器
	clock_t Begin = clock();

	//接收数据包
	Head recv;
	int len = sizeof(ServerAddr);
	while (true) {
		//接收到数据包，首先清空缓冲区
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(Client, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			break;
			//验证消息类型、校验和、序列号
			if (CheckSum((u_short*)&recv, sizeof(recv)) == 0 && recv.type == ACK && recv.seq == Seq) {
				//接收到正确的数据包
				//打印日志并退出
				cout << "[ACK]Server:\t\t" << "Seq=" << int(recv.seq) << "\tCheckSum=" << recv.checksum << endl;
				break;
			}
			else {
				//接收到的数据包出错，需要重传
				cout << "第" << send.seq << "号数据包差错重传..." << endl;
				if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				//重新计时
				Begin = clock();
			}
		}
		else if (clock() - Begin > MAX_WAIT_TIME) {
			return true;
			//计时器超时，需要重传
			cout << "第" << int(send.seq) << "号数据包超时重传..." << endl;
			if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			//重新计时
			Begin = clock();
		}
	}

	return true;
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

//读文件到缓冲区
void ReadFile() {
	int option;
	//选择要上传文件
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
		FileName = "helloworld.txt";
		break;
	default:
		cout << "请输入正确的选项!" << endl;
	}

	//打开文件
	ifstream fin(FileName.c_str(), ifstream::in | ios::binary);
	if (!fin) {
		cout << "Error: cannot open file!" << endl;
	}

	//获取文件大小
	fin.seekg(0, fin.end);
	FileSize = fin.tellg();
	fin.seekg(0, fin.beg);

	//读文件到缓冲区
	cout << FileSize;
	memset(Buffer, 0, FileSize);
	fin.read(Buffer, FileSize);
	fin.close();
}

void sendData(char* Buffer, int FileSize) {
	// 实际发送窗口取决于接收通告窗口和拥塞控制窗口中的较小值
	int window = 0;
	HANDLE Thread1 = CreateThread(NULL, NULL, recvThread, LPVOID(&Client), 0, NULL);
	HANDLE Thread2 = CreateThread(NULL, NULL, timer, LPVOID(NULL), 0, NULL);
	int sentLen = 0;					//已经发送的长度
	u_int size = MAX_SEND_SIZE;			//发送数据包size
	bool last = 0;						//判断是否是最后一个数据包
	base = Seq;

	while (1) {
		//更新窗口
		if (rwnd < cwnd)
			window = rwnd;
		else
			window = cwnd;

		//需要超时重传
		if (resend == true) {
			sentLen = lenCopy;
			Seq = base;
			resend = false;
		}

		if (((base + window) < 256 && (int)Seq < base + window) ||
			((base + window) >= 256 && ((int)Seq >= base || (int)Seq < (base + window) % 256))) {
			if (sentLen + MAX_SEND_SIZE > FileSize) {
				size = FileSize - sentLen;
				last = 1;
			}
			sendPackage(Buffer + sentLen, size, Seq, window, last);
			Seq++;
			sentLen += size;
			if (sentLen == FileSize) {
				CloseHandle(Thread1);
				CloseHandle(Thread2);
				break;
			}
		}
		// 若下一个序列号不在窗口内，等待ACK
		else {
			cout << "Waiting for window!" << endl;
			wait = true;
			while (wait & !resend) {
				//Sleep(100);
			}
		}
	}
}



//标志位restart、resend
//timer线程函数，管理计时器
DWORD WINAPI timer(LPVOID lparam) {
	clock_t start = clock();
	while (true) {
		//是否重新开始计时
		if (restart) {
			start = clock();
			restart = false;
		}
		//是否超时重传
		if (clock() - start > MAX_WAIT_TIME) {
			resend = true;
			start = clock();

			// 超时后阈值减半，窗口大小为1
			//回到慢启动阶段
			ssthresh = cwnd / 2;
			cwnd = 1;
			dupAck = 0;
			renoState = 0;
		}
	}
	return 0;
}

//接收ACK线程
DWORD WINAPI recvThread(LPVOID IpParameter) {
	//线程socket sockSender 
	SOCKET sockSender = *(SOCKET*)IpParameter;
	int len = sizeof(SOCKADDR);
	
	//接收数据包recv
	Head recv;
	while (true) {
		//接收到数据包
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(sockSender, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			// 验证校验和、类型字段
			if (recv.type == ACK && !CheckSum((u_short*)&recv, sizeof(recv))) {
				lenCopy += (recv.seq - base) * MAX_SEND_SIZE;
				base = recv.seq;
				//清除标志位
				restart = true;
				resendCount = 0;
				wait = false;

				// 慢启动
				if (renoState == 0) {
					// 判断是否为new ACK，是的话更新窗口空间
					if (recv.seq == lastAck) {
						dupAck++;
					}
					else {
						dupAck = 0;
						lastAck = recv.seq;
						cwnd++;
					}
					// 窗口大小超过阈值，进入拥塞避免阶段
					if (cwnd >= ssthresh)
						renoState = 1;
					// 重复ACK超过3次，进入快速恢复阶段
					if (dupAck == 3) {
						cout << "快速恢复..." << endl;
						resend = 1;
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3;
						renoState = 2;
					}
				}
				// 拥塞避免
				else if (renoState == 1) {
					if (recv.seq == lastAck) {
						dupAck++;
					}
					else {
						dupAck = 0;
						lastAck = recv.seq;
						cwnd += 1.0 / cwnd;
					}
					// 重复ACK超过3次，进入快速恢复阶段
					if (dupAck == 3) {
						cout << "快速恢复..." << endl;
						resend = 1;
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3;
						renoState = 2;
					}
				}
				// 快速恢复
				else {
					if (recv.seq == lastAck) {
						cwnd++;
					}
					//新RENO算法
					// 接收到new ACK，需要判断是否发送的消息都被接收
					// 如果不是，保持快速恢复；如果是，进入拥塞避免阶段
					else {
						if (recv.seq == Seq) {
							renoState = 1;
							lastAck = recv.seq;
						}
					}
				}
			}
		}
	}
	return 0;
}

//发送数据包函数
void sendPackage(char* message, int length, u_char seq, u_char window, bool last = 0) {
	//构造数据包
	memset(sendBuf, 0, sizeof(sendBuf));
	Head send;
	send.type = SEND;
	if (last) {
		send.type = OVER;
	}
	send.length = length;
	send.window = window;
	send.seq = seq;
	memcpy(sendBuf, &send, sizeof(Head));
	memcpy(sendBuf + sizeof(Head), message, length);
	u_short checksum = CheckSum((u_short*)sendBuf, sizeof(Head) + length);
	send.checksum = checksum;
	memcpy(sendBuf, &send, sizeof(Head));

	//发送数据包
	if (sendto(Client, sendBuf, sizeof(Head) + length, 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
	}
	cout << "[SEND]" << "Client: " << "Seq = " << int(send.seq) << "  Window = " << int(send.window)
		<< "  cwnd=" << cwnd << "  ssthresh=" << ssthresh << "  Length = " << send.length << "  CheckSum = " << int(send.checksum) << endl;
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
void Packet(Head& head, u_char type, u_char seq, u_char window = WINDOW) {
	memset(&head, 0, sizeof(head));
	head.type = type;
	head.seq = seq;
	head.window = window;
	head.checksum = CheckSum((u_short*)&head, sizeof(head));
}
