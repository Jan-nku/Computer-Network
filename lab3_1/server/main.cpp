#include "Server.h"
using namespace std;
int main() {
	init();
	printf("connecting...\n");
	if (!ShakeHand()) {
		printf("Handshake failed!\n");
		return 0;
	}
	printf("Handshake succeeded!\n");
	FileName = RecvFileName();
	recvData();
	ofstream fout(FileName.c_str(), ofstream::out | ios::binary);
	if (!fout) {
		cout << "Error: cannot open file!" << endl;
		return 0;
	}
	cout << "���ܵ����ݴ�С:" << FileSize << " Bytes" << endl;
	fout.write(FileBuf, FileSize);
	fout.close();
	if (!WaveHand()) {
		cout << "����ʧ��" << endl;
		return 0;
	}
	cout << "���ֳɹ�" << endl;
	release();
	return 0;
}


