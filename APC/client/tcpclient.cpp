#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#pragma comment(lib, "wsock32")  //加载 ws2_32.dll

int main() {
	//初始化DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//创建套接字
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//向服务器发起请求
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
	sockAddr.sin_family = PF_INET;
	sockAddr.sin_addr.s_addr = inet_addr("67.218.148.94");
	sockAddr.sin_port = htons(80);
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));

	Sleep(10000);
	int c = 1;
	while (c++)
	{
		printf("%d\t ",c);
		//接收服务器传回的数据
		char* Header=(char*) "GET / HTTP/1.1\r\n\r\n";
		char szBuffer[2000] = { 0 };
		send(sock, Header, strlen(Header),0);
		printf("send well\n");
		recv(sock, szBuffer, 1000, NULL);

		//输出接收到的数据
		printf("Message form server: %s\n", szBuffer);
		Sleep(2000);
	}
	//关闭套接字
	closesocket(sock);

	//终止使用 DLL
	WSACleanup();

	system("pause");
	return 0;
}