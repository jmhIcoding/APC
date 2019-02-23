#pragma once
#include <windows.h>
#include <string.h>
#include <stdio.h>
#pragma comment(lib,"WS2_32")
typedef struct _rule
{
	char direct=0;//方向。0:outbound,1:inbound
	char proto_type=0;//协议类型 0:tcp,1:udp;
	unsigned short port;//端口号;都是本地的port
	int hash()
	{
		return  direct << 24 + proto_type << 20 + port;
	}
	void tostring(char *buf, int buflen)
	{
		memset(buf, 0, buflen);
		switch (direct)
		{
			case 1:
				if (proto_type == 1)
				{
					sprintf(buf, "inbound and udp and udp.SrcPort==%d", port);
				}
				else
				{
					sprintf(buf, "inbound and tcp and tcp.SrcPort == %d", port);
				}
			break;
		default:
			if (proto_type == 1)
			{
				sprintf(buf, "outbound and udp and udp.SrcPort==%d", port);
			}
			else
			{
				sprintf(buf, "outbound and tcp and tcp.SrcPort == %d", port);
			}
			break;
		}
	}
}filter_rule;

void get_sock_local_info(SOCKET socket_fd, filter_rule * rule)
{
	struct sockaddr_in localaddr;
	int localaddrLen;
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(socketVersion, &wsaData);
	int ret = getsockname(socket_fd, (struct sockaddr *)&localaddr, &localaddrLen);
	WSACleanup();
	rule->port=ntohs(localaddr.sin_port);
	//printf("sin_famility:%d\n", localaddr.sin_family);
	//rule->proto_type = localaddr.sin_family == SOCK_STREAM ? 0 : 1;//0:tcp,1:udp
	//rule->proto_type = 0;//暂时写死为tcp
	//fprintf(stderr, "error: %d,%d\n", ret, WSAGetLastError());
}