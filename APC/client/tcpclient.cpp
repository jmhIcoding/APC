#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#pragma comment(lib, "wsock32")  //���� ws2_32.dll

int main() {
	//��ʼ��DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//�����׽���
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//���������������
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  //ÿ���ֽڶ���0���
	sockAddr.sin_family = PF_INET;
	sockAddr.sin_addr.s_addr = inet_addr("67.218.148.94");
	sockAddr.sin_port = htons(80);
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));

	Sleep(10000);
	int c = 1;
	while (c++)
	{
		printf("%d\t ",c);
		//���շ��������ص�����
		char* Header=(char*) "GET / HTTP/1.1\r\n\r\n";
		char szBuffer[2000] = { 0 };
		send(sock, Header, strlen(Header),0);
		printf("send well\n");
		recv(sock, szBuffer, 1000, NULL);

		//������յ�������
		printf("Message form server: %s\n", szBuffer);
		Sleep(2000);
	}
	//�ر��׽���
	closesocket(sock);

	//��ֹʹ�� DLL
	WSACleanup();

	system("pause");
	return 0;
}