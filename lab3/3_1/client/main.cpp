#include "Client.h"
using namespace std;
int main()
{
	init();
	printf("connecting...\n");
	if (!ShakeHand()) {
		printf("Handshake failed!\n");
		return 0;
	}
	printf("Handshake succeeded!\n");
	ReadFile();
	SendFileName(FileName);
	Send_Message(FileBuf, FileSize);
	cout << "send over!" << endl;
	cout << "发送数据:" << FileSize << " Bytes" << endl;
	cout << "发送时间" << (float)SendTime / 1000 << "s" << endl;
	cout << "吞吐率" << FileSize / float(SendTime) << "KB/s" << endl;
	if (!WaveHand()) {
		cout << "挥手失败" << endl;
		return 0;
	}
	cout << "挥手成功" << endl;
	release();
	system("pause");
	return 0;

}