#include"client.h"
using namespace std;


int main()
{
	//初始化
	init();

	//握手
	printf("connecting...\n");
	if (!ShakeHand()) {
		printf("Handshake failed!\n");
		return 0;
	}
	printf("Handshake succeeded!\n");
	
	//读文件	
	ReadFile();
	//sendData((char*)FileName.c_str(), FileName.length());

	clock_t SendTime = clock();
	sendData(Buffer, FileSize);
	SendTime = clock() - SendTime;

	//日志打印
	cout << "send over!" << endl;
	cout << "发送数据:" << FileSize << " Bytes" << endl;
	cout << "发送时间" << (float)SendTime / 1000 << "s" << endl;
	cout << "吞吐率" << FileSize / float(SendTime) << "KB/s" << endl;
	
	//挥手
	cout << "Wave Hand..." << endl;
	if (!WaveHand()) {
		cout << "挥手失败" << endl;
		return 0;
	}
	cout << "挥手成功" << endl;

	//释放资源
	release();
	system("pause");
	return 0;

}