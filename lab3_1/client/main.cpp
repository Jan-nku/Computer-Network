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
	cout << "��������:" << FileSize << " Bytes" << endl;
	cout << "����ʱ��" << (float)SendTime / 1000 << "s" << endl;
	cout << "������" << FileSize / float(SendTime) << "KB/s" << endl;
	if (!WaveHand()) {
		cout << "����ʧ��" << endl;
		return 0;
	}
	cout << "���ֳɹ�" << endl;
	release();
	system("pause");
	return 0;

}