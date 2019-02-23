/*
���ļ���Ŀ�ģ�
	hook Ŀ����̵�tcp��send����,�Լ�udp��sendto ����;����dll��
	Ȼ������д����,ʹ��createProcessWithDll,����Ŀ����̡�
*/
#include "netfilter.h"
#pragma comment(lib,"WS2_32")
#pragma comment(lib,"detours")
#pragma comment(lib,"syelog")
#pragma comment(lib,"WinDivert")
#include <windows.h>
#include <detours.h>
#include "util.h"
#include <map>;
using namespace std;
FILE * fp;
map<int, netfilter *> rule_to_filter_instance;

int str2hash(char * str, int len)
{
	int rst = 1;
	int mod = 1e9 + 7,prime = 19950817;
	for (int i = 0; i < len; i++)
	{
		rst = (rst * prime + str[i]) % mod;
	}
	return rst;
}
//��ʵ�ķ��ͺ���:tcp send,udp sendto,������������ϵͳ���Դ�����ȷ����
SOCKET (WINAPI* TrueSocket)( int af,int type, int protocol)=socket;
int (WINAPI * TrueConnect)(SOCKET s, const struct sockaddr  *name,int namelen) = connect;
int (WINAPI * TrueSend)(SOCKET s, const char * buf, int len, int flags) = send;
int (WINAPI* TrueSendto)(SOCKET s,  const char  * buf, int len,  int flags,  const struct sockaddr  *to, int tolen) = sendto;
int (WINAPI * TrueClosesocket)( SOCKET s) = closesocket;
//α�캯���ĺ���
int WINAPI  SelfConnect(SOCKET s, const struct sockaddr  *name, int namelen)
{
	int rst =connect(s, name, namelen);
	fprintf(fp, "call self connect .\n");
	filter_rule rule;
	get_sock_local_info(s, &rule);
	char rulebuf[32] = { 0 };
	int hash = rule.hash();
	if (rule_to_filter_instance.find(hash) == rule_to_filter_instance.end())
		//�����ڶ�Ӧ�Ĺ���,�����µĶ���
	{
		rule.tostring((char *)rulebuf, 32);
		rule_to_filter_instance[hash] = new netfilter((char*)rulebuf, filter_type_modify);
		fprintf(fp, rulebuf);
		fprintf(fp, "\n");

	}
	fflush(fp);
	return rst;
}
int WINAPI SelfSocket(int af, int type, int protocol)
{
	fprintf(fp, "call SelfSocket...\n");
	fflush(fp);
	return TrueSocket(af, type, protocol);
}
int WINAPI SelfSend( SOCKET s,  const char  * buf,  int len,  int flags)
//�Զ����tcp send ����
{
	
	filter_rule rule;
	get_sock_local_info(s, &rule);
	char rulebuf[32] = { 0 };
	int hash = rule.hash();
	fprintf(fp, "call SelfSend...\n");
	if (rule_to_filter_instance.find(hash) == rule_to_filter_instance.end())
		//�����ڶ�Ӧ�Ĺ���,�����µĶ���
	{
		rule.tostring((char *)rulebuf, 32);
		rule_to_filter_instance[hash] = new netfilter((char*)rulebuf, filter_type_modify);
		fprintf(fp, rulebuf);
		fprintf(fp, "\n");
		
	}
	fflush(fp);
	return TrueSend(s, buf, len, flags);
}
int WINAPI SelfSendto(SOCKET s,  const char * buf,  int len, int flags,  const struct sockaddr  *to, int tolen)
//�Զ����udp sendto ����
{
	filter_rule rule;
	get_sock_local_info(s, &rule);
	char rulebuf[32] = { 0 };
	int hash = rule.hash();
	fprintf(fp, "call self send to\n");
	if (rule_to_filter_instance.find(hash) == rule_to_filter_instance.end())
		//�����ڶ�Ӧ�Ĺ���,�����µĶ���
	{
		rule.tostring((char *)rulebuf, 32);
		rule_to_filter_instance[hash] = new netfilter((char*)rulebuf, filter_type_modify);
		fprintf(fp, rulebuf);
		fprintf(fp, "\n");
		fflush(fp);
	}
	fflush(fp);
	return TrueSendto(s, buf, len, flags, to, tolen);
}
int WINAPI SelfClosesocket(SOCKET s)
{
	filter_rule rule;
	get_sock_local_info(s, &rule);
	int hash = rule.hash();
	if (rule_to_filter_instance.find(hash) != rule_to_filter_instance.end())
		//���ڶ�Ӧ�Ĺ���
	{
		delete rule_to_filter_instance[hash];
	}
	return TrueClosesocket(s);
}
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}
	if (dwReason == DLL_PROCESS_ATTACH) {
		//Send��ע��
		
		fp = fopen("C:\\Users\\jmh081701\\Documents\\APC\\APC\\x64\\Debug\\log.txt", "w");
		fprintf(fp, "DLL ATTACH\n");
		fflush(fp);
	
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueSocket, SelfSocket);
		DetourAttach(&(PVOID&)TrueConnect, SelfConnect);

		DetourAttach(&(PVOID&)TrueSend, SelfSend);
;
		//Sendto��ע��
		DetourAttach(&(PVOID&)TrueSendto, SelfSendto);
		//closesocket��ע��
		DetourAttach(&(PVOID&)TrueClosesocket, SelfClosesocket);
		int ret = DetourTransactionCommit();
		fprintf(fp, "Detour Trans:%d\n", ret == NO_ERROR);
		fflush(fp);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		//Send�Ļָ�
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueSocket, SelfSocket);
		DetourDetach(&(PVOID&)TrueConnect, SelfConnect);
		DetourDetach(&(PVOID&)TrueSend, SelfSend);
		//Sendto�Ļָ�
		DetourDetach(&(PVOID&)TrueSendto, SelfSendto);
		//closeSocket�Ļָ�
		DetourDetach(&(PVOID&)TrueClosesocket, SelfClosesocket);
		DetourTransactionCommit();
		fclose(fp);
	}
	return TRUE;
}
_declspec(dllexport) int go(int)
{
	return 0;
}