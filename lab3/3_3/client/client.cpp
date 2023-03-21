#include"client.h"
//����ȫ�ֱ���
//SocketͨѶ
WSAData wsaData;
SOCKET Client;
SOCKADDR_IN ServerAddr;

//���к�
u_char Seq;

//�ļ���д
string FileName;			//�ļ���
u_int FileSize;				//�ļ���С
char Buffer[100000000];		//�ļ�������
char sendBuf[20000];		//�������ݻ�����

//GBN��Ҫʹ�õ�ȫ�ֱ���
int resendCount = 0;		// �ش�����
int base = 2;				// base֮ǰ���к��ۼ�ȷ��
bool resend = false;		// �Ƿ�ʱ�ش�
bool restart = false;		// �Ƿ����¿�ʼ��ʱ
bool wait = false;			// �Ƿ��򴰿ڲ�������ȴ�
long long lenCopy = 0;		// �ļ�����ƫ�����Ŀ��������ش�ʱʹ��

//ӵ������
const int rwnd = 10;		// ����ͨ�洰�ڴ�С���̶�ֵ
double cwnd = 1;			// ӵ�����ڴ�С
int ssthresh = 8;			// ��ֵ
unsigned char lastAck = 0;	// ��һ��ACK���к�
int dupAck = 0;				// �ظ��յ���ACK����
int renoState = 0;			// RENO״̬����״̬��0Ϊ��������1Ϊӵ�����ƣ�2Ϊ���ٻָ�


//����
//��ʼ�����ͷź���
void init() {
	//WSAStartup
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw("WSADATA init false! error code : %d", WSAGetLastError());
	}
	printf("WSADATA init success!\n");

	//����ServerAddr
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = inet_addr(IP);
	
	//����Client Socket
	Client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Client == INVALID_SOCKET) {
		closesocket(Client);
		throw("socket of client invalid!");
	}
	printf("create socket of client success!\n");
	
	//����Ϊ������ģʽ  
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

//�������֡����λ���
bool ShakeHand() {
	//��װ���ݰ�send
	Head send;
	send.seq = Seq;
	send.type = SYN;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));

	//�������ݰ�
	if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}

	//���к���1
	Seq++;
	//��־��ӡ
	cout << "[SYN]Client:\t\t" << "Seq=" << int(send.seq) << "\tCheckSum=" << send.checksum << endl;

	//������ʱ������ⳬʱ
	clock_t Begin = clock();

	//�������ݰ�recv
	Head recv;
	int len = sizeof(ServerAddr);
	while (true) {
		//�������ݰ����������������
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(Client, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			//��֤��Ϣ���͡�У��͡����к�
			if (!CheckSum((u_short*)&recv, sizeof(recv)) && recv.type == (SYN | ACK) && recv.seq == Seq) {
				//���յ���Ҫ�����ݰ�
				//��ӡ��־���˳�
				cout << "[SYN, ACK]Server:\t\t" << "Seq = " << int(recv.seq) << "\tCheckSum=" << send.checksum << endl;
				break;
			}
			else {
				//���յ������ݰ��д�����Ҫ�ش�
				cout << "��" << send.seq << "�����ݰ�����ش�..." << endl;
				if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				//���¼�ʱ
				Begin = clock();
			}
		}
		//��ʱ����ʱ
		else if (clock() - Begin > MAX_WAIT_TIME) {
			//�ش����ݰ�
			cout << "��" << int(send.seq) << "�����ݰ���ʱ�ش�..." << endl;
			if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			//���¼�ʱ
			Begin = clock();
		}
	}

	//���յ��������ݰ����˳�ѭ��
	//��װ���ݰ�
	send.seq = Seq;
	send.type = ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(send));

	//�������ݰ�
	if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	//���к���1
	Seq++;
	//��־��ӡ
	cout << "[ACK]Client:\t\t" << "Seq=" << int(send.seq) << "\tCheckSum=" << send.checksum << endl;
	return true;
}
bool WaveHand() {
	//��װ���ݰ�
	Head send;
	send.seq = Seq;
	send.type = FIN | ACK;
	send.checksum = 0;
	send.checksum = CheckSum((u_short*)&send, sizeof(Head));
	
	//�������ݰ�
	if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
		return false;
	}
	
	//���к���1
	Seq++;
	//��ӡ��־
	cout << "[FIN,ACK]Client:\t\t" << "Seq=" << int(send.seq) << "\tCheckSum=" << send.checksum << endl;

	//������ʱ��
	clock_t Begin = clock();

	//�������ݰ�
	Head recv;
	int len = sizeof(ServerAddr);
	while (true) {
		//���յ����ݰ���������ջ�����
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(Client, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			break;
			//��֤��Ϣ���͡�У��͡����к�
			if (CheckSum((u_short*)&recv, sizeof(recv)) == 0 && recv.type == ACK && recv.seq == Seq) {
				//���յ���ȷ�����ݰ�
				//��ӡ��־���˳�
				cout << "[ACK]Server:\t\t" << "Seq=" << int(recv.seq) << "\tCheckSum=" << recv.checksum << endl;
				break;
			}
			else {
				//���յ������ݰ�������Ҫ�ش�
				cout << "��" << send.seq << "�����ݰ�����ش�..." << endl;
				if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
					throw("Error: fail to send messages!");
					return false;
				}
				//���¼�ʱ
				Begin = clock();
			}
		}
		else if (clock() - Begin > MAX_WAIT_TIME) {
			return true;
			//��ʱ����ʱ����Ҫ�ش�
			cout << "��" << int(send.seq) << "�����ݰ���ʱ�ش�..." << endl;
			if (sendto(Client, (char*)&send, sizeof(send), 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
				throw("Error: fail to send messages!");
				return false;
			}
			//���¼�ʱ
			Begin = clock();
		}
	}

	return true;
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

//���ļ���������
void ReadFile() {
	int option;
	//ѡ��Ҫ�ϴ��ļ�
	cout << "ѡ��Ҫ�ϴ����ļ���" << endl;
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
		cout << "��������ȷ��ѡ��!" << endl;
	}

	//���ļ�
	ifstream fin(FileName.c_str(), ifstream::in | ios::binary);
	if (!fin) {
		cout << "Error: cannot open file!" << endl;
	}

	//��ȡ�ļ���С
	fin.seekg(0, fin.end);
	FileSize = fin.tellg();
	fin.seekg(0, fin.beg);

	//���ļ���������
	cout << FileSize;
	memset(Buffer, 0, FileSize);
	fin.read(Buffer, FileSize);
	fin.close();
}

void sendData(char* Buffer, int FileSize) {
	// ʵ�ʷ��ʹ���ȡ���ڽ���ͨ�洰�ں�ӵ�����ƴ����еĽ�Сֵ
	int window = 0;
	HANDLE Thread1 = CreateThread(NULL, NULL, recvThread, LPVOID(&Client), 0, NULL);
	HANDLE Thread2 = CreateThread(NULL, NULL, timer, LPVOID(NULL), 0, NULL);
	int sentLen = 0;					//�Ѿ����͵ĳ���
	u_int size = MAX_SEND_SIZE;			//�������ݰ�size
	bool last = 0;						//�ж��Ƿ������һ�����ݰ�
	base = Seq;

	while (1) {
		//���´���
		if (rwnd < cwnd)
			window = rwnd;
		else
			window = cwnd;

		//��Ҫ��ʱ�ش�
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
		// ����һ�����кŲ��ڴ����ڣ��ȴ�ACK
		else {
			cout << "Waiting for window!" << endl;
			wait = true;
			while (wait & !resend) {
				//Sleep(100);
			}
		}
	}
}



//��־λrestart��resend
//timer�̺߳����������ʱ��
DWORD WINAPI timer(LPVOID lparam) {
	clock_t start = clock();
	while (true) {
		//�Ƿ����¿�ʼ��ʱ
		if (restart) {
			start = clock();
			restart = false;
		}
		//�Ƿ�ʱ�ش�
		if (clock() - start > MAX_WAIT_TIME) {
			resend = true;
			start = clock();

			// ��ʱ����ֵ���룬���ڴ�СΪ1
			//�ص��������׶�
			ssthresh = cwnd / 2;
			cwnd = 1;
			dupAck = 0;
			renoState = 0;
		}
	}
	return 0;
}

//����ACK�߳�
DWORD WINAPI recvThread(LPVOID IpParameter) {
	//�߳�socket sockSender 
	SOCKET sockSender = *(SOCKET*)IpParameter;
	int len = sizeof(SOCKADDR);
	
	//�������ݰ�recv
	Head recv;
	while (true) {
		//���յ����ݰ�
		memset(&recv, 0, sizeof(recv));
		if (recvfrom(sockSender, (char*)&recv, sizeof(recv), 0, (SOCKADDR*)&ServerAddr, &len) > 0) {
			// ��֤У��͡������ֶ�
			if (recv.type == ACK && !CheckSum((u_short*)&recv, sizeof(recv))) {
				lenCopy += (recv.seq - base) * MAX_SEND_SIZE;
				base = recv.seq;
				//�����־λ
				restart = true;
				resendCount = 0;
				wait = false;

				// ������
				if (renoState == 0) {
					// �ж��Ƿ�Ϊnew ACK���ǵĻ����´��ڿռ�
					if (recv.seq == lastAck) {
						dupAck++;
					}
					else {
						dupAck = 0;
						lastAck = recv.seq;
						cwnd++;
					}
					// ���ڴ�С������ֵ������ӵ������׶�
					if (cwnd >= ssthresh)
						renoState = 1;
					// �ظ�ACK����3�Σ�������ٻָ��׶�
					if (dupAck == 3) {
						cout << "���ٻָ�..." << endl;
						resend = 1;
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3;
						renoState = 2;
					}
				}
				// ӵ������
				else if (renoState == 1) {
					if (recv.seq == lastAck) {
						dupAck++;
					}
					else {
						dupAck = 0;
						lastAck = recv.seq;
						cwnd += 1.0 / cwnd;
					}
					// �ظ�ACK����3�Σ�������ٻָ��׶�
					if (dupAck == 3) {
						cout << "���ٻָ�..." << endl;
						resend = 1;
						ssthresh = cwnd / 2;
						cwnd = ssthresh + 3;
						renoState = 2;
					}
				}
				// ���ٻָ�
				else {
					if (recv.seq == lastAck) {
						cwnd++;
					}
					//��RENO�㷨
					// ���յ�new ACK����Ҫ�ж��Ƿ��͵���Ϣ��������
					// ������ǣ����ֿ��ٻָ�������ǣ�����ӵ������׶�
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

//�������ݰ�����
void sendPackage(char* message, int length, u_char seq, u_char window, bool last = 0) {
	//�������ݰ�
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

	//�������ݰ�
	if (sendto(Client, sendBuf, sizeof(Head) + length, 0, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1) {
		throw("Error: fail to send messages!");
	}
	cout << "[SEND]" << "Client: " << "Seq = " << int(send.seq) << "  Window = " << int(send.window)
		<< "  cwnd=" << cwnd << "  ssthresh=" << ssthresh << "  Length = " << send.length << "  CheckSum = " << int(send.checksum) << endl;
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
void Packet(Head& head, u_char type, u_char seq, u_char window = WINDOW) {
	memset(&head, 0, sizeof(head));
	head.type = type;
	head.seq = seq;
	head.window = window;
	head.checksum = CheckSum((u_short*)&head, sizeof(head));
}
