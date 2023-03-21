#include"server.h"
using namespace std;

int main() {
	//初始化
	init();

	//握手
	printf("connecting...\n");
	if (!ShakeHand()) {
		printf("Handshake failed!\n");
		return 0;
	}
	printf("Handshake succeeded!\n");
	
	//接收文件名
	//recvData();
	//memcpy(FileName, Buffer, FileSize);
	
	//接收文件内容
	recvData();

	//打开文件
	memcpy(FileName, string("1.jpg").c_str(), 5);
	ofstream fout(FileName, ofstream::out | ios::binary);
	if (!fout) {
		cout << "Error: cannot open file!" << endl;
		return 0;
	}
	cout << "接受的数据大小:" << FileSize << " Bytes" << endl;
	
	//写缓冲区
	fout.write(Buffer, FileSize);
	fout.close();

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

