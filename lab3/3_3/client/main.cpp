#include"client.h"
using namespace std;


int main()
{
	//��ʼ��
	init();

	//����
	printf("connecting...\n");
	if (!ShakeHand()) {
		printf("Handshake failed!\n");
		return 0;
	}
	printf("Handshake succeeded!\n");
	
	//���ļ�	
	ReadFile();
	//sendData((char*)FileName.c_str(), FileName.length());

	clock_t SendTime = clock();
	sendData(Buffer, FileSize);
	SendTime = clock() - SendTime;

	//��־��ӡ
	cout << "send over!" << endl;
	cout << "��������:" << FileSize << " Bytes" << endl;
	cout << "����ʱ��" << (float)SendTime / 1000 << "s" << endl;
	cout << "������" << FileSize / float(SendTime) << "KB/s" << endl;
	
	//����
	cout << "Wave Hand..." << endl;
	if (!WaveHand()) {
		cout << "����ʧ��" << endl;
		return 0;
	}
	cout << "���ֳɹ�" << endl;

	//�ͷ���Դ
	release();
	system("pause");
	return 0;

}