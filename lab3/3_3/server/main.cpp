#include"server.h"
using namespace std;

int main() {
	//��ʼ��
	init();

	//����
	printf("connecting...\n");
	if (!ShakeHand()) {
		printf("Handshake failed!\n");
		return 0;
	}
	printf("Handshake succeeded!\n");
	
	//�����ļ���
	//recvData();
	//memcpy(FileName, Buffer, FileSize);
	
	//�����ļ�����
	recvData();

	//���ļ�
	memcpy(FileName, string("1.jpg").c_str(), 5);
	ofstream fout(FileName, ofstream::out | ios::binary);
	if (!fout) {
		cout << "Error: cannot open file!" << endl;
		return 0;
	}
	cout << "���ܵ����ݴ�С:" << FileSize << " Bytes" << endl;
	
	//д������
	fout.write(Buffer, FileSize);
	fout.close();

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

