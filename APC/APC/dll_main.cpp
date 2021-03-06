/*
本文件的目的：
	hook 目标进程的tcp的send函数,以及udp的sendto 函数;生成dll。
	然后另外写程序,使用createProcessWithDll,启动目标进程。
*/
#include "netfilter.h"
#pragma comment(lib,"wsock32")
#pragma comment(lib,"detours")
#pragma comment(lib,"syelog")
#pragma comment(lib,"WinDivert")
#pragma comment (lib,"windivert.lib")
#include <windows.h>
#include <detours.h>
#include "util.h"
#include <map>;
using namespace std;
FILE * fp;
map<int, netfilter *> * rule_to_filter_instance;

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
//真实的发送函数:tcp send,udp sendto,这两个函数是系统级自带的正确函数
SOCKET (WINAPI* TrueSocket)( int af,int type, int protocol)=socket;
int (WINAPI * TrueConnect)(SOCKET s, const struct sockaddr  *name,int namelen) = connect;
int (WINAPI * TrueSend)(SOCKET s, const char * buf, int len, int flags) = send;
int (WINAPI* TrueSendto)(SOCKET s,  const char  * buf, int len,  int flags,  const struct sockaddr  *to, int tolen) = sendto;
int (WINAPI * TrueClosesocket)( SOCKET s) = closesocket;
//伪造函数的函数
int WINAPI  SelfConnect(SOCKET s, const struct sockaddr  *name, int namelen)
{
	int rst =connect(s, name, namelen);
	fprintf(fp, "call self connect .\n");
	filter_rule rule;
	get_sock_local_info(s, &rule);
	char rulebuf[64] = { 0 };
	int hash = rule.hash();
	if (rule_to_filter_instance->find(hash) == rule_to_filter_instance->end())
		//不存在对应的规则,生成新的对象
	{
		rule.tostring((char *)rulebuf, 64);
		(*rule_to_filter_instance)[hash] = new netfilter((char*)rulebuf, filter_type_modify);
		fprintf(fp, rulebuf);
		fprintf(fp, "\n");

	}
	fflush(fp);
	return rst;
}
SOCKET WINAPI SelfSocket(int af, int type, int protocol)
{
	fprintf(fp, "call SelfSocket...\n");
	
	SOCKET s= TrueSocket(af, type, protocol);
	
	filter_rule rule;
	get_sock_local_info(s, &rule);
	
	char rulebuf[64] = { 0 };
	int hash = rule.hash();
	
	if (rule_to_filter_instance->find(hash) == rule_to_filter_instance->end())
		//不存在对应的规则,生成新的对象
	{

		rule.tostring((char *)rulebuf, 64);
		(*rule_to_filter_instance)[hash] = new netfilter((char*)rulebuf, filter_type_modify);
		fprintf(fp, rulebuf);
		fprintf(fp, "\n");

	}
	fflush(fp);
	fprintf(fp, "create socket %d .\n ", s);
	fflush(fp);
	return s;
}
int WINAPI SelfSend( SOCKET s,  const char  * buf,  int len,  int flags)
//自定义的tcp send 函数
{
	
	fprintf(fp, "call SelfSend...\n");
	fflush(fp);
	filter_rule rule;
	get_sock_local_info(s, &rule);
	char rulebuf[128] = { 0 };
	int hash = rule.hash();
	rule.tostring((char *)rulebuf, 128);
	if (rule_to_filter_instance->find(hash) == rule_to_filter_instance->end())
		//不存在对应的规则,生成新的对象
	{
		(*rule_to_filter_instance)[hash] = new netfilter((char*)rulebuf, filter_type_modify);
		fprintf(fp, rulebuf);
		fprintf(fp, "\n");
		
	}
	else
	{

		fprintf(fp,"Existing rule:%s ...\n",rulebuf);
	}
	fflush(fp);
	return TrueSend(s, buf, len, flags);
}
int WINAPI SelfSendto(SOCKET s,  const char * buf,  int len, int flags,  const struct sockaddr  *to, int tolen)
//自定义的udp sendto 函数
{
	fprintf(fp, "call self send to\n");
	fflush(fp);
	filter_rule rule;
	get_sock_local_info(s, &rule);
	char rulebuf[64] = { 0 };
	int hash = rule.hash();
	
	if (rule_to_filter_instance->find(hash) == rule_to_filter_instance->end())
		//不存在对应的规则,生成新的对象
	{
		rule.tostring((char *)rulebuf, 64);
		(*rule_to_filter_instance)[hash] = new netfilter((char*)rulebuf, filter_type_modify);
		fprintf(fp, rulebuf);
		fprintf(fp, "\n");
		fflush(fp);
	}
	fflush(fp);
	return TrueSendto(s, buf, len, flags, to, tolen);
}
int WINAPI SelfClosesocket(SOCKET s)
{
	fprintf(fp, "Self CloseSocket....\n");
	fflush(fp);
	filter_rule rule;
	get_sock_local_info(s, &rule);
	int hash = rule.hash();
	if (rule_to_filter_instance->find(hash) != rule_to_filter_instance->end())
		//存在对应的规则
	{
		delete (*rule_to_filter_instance)[hash];
	}
	fprintf(fp, "Self CloseSocket well....\n");
	fflush(fp);
	return TrueClosesocket(s);
}
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}
	if (dwReason == DLL_PROCESS_ATTACH) {
		//Send的注入
		
		fp = fopen("C:\\Users\\jmh081701\\Documents\\APC\\APC\\x64\\Debug\\log.txt", "w");
		rule_to_filter_instance = new map<int, netfilter *>();
		fprintf(fp, "DLL ATTACH\n");
		fflush(fp);
		//MessageBox(NULL, "Inject Sucess", "TIPS", NULL);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueSocket, SelfSocket);
		DetourAttach(&(PVOID&)TrueConnect, SelfConnect);

		DetourAttach(&(PVOID&)TrueSend, SelfSend);
;
		//Sendto的注入
		DetourAttach(&(PVOID&)TrueSendto, SelfSendto);
		//closesocket的注入
		DetourAttach(&(PVOID&)TrueClosesocket, SelfClosesocket);
		int ret = DetourTransactionCommit();
		fprintf(fp, "Detour Trans:%d\n", ret == NO_ERROR);
		fflush(fp);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		//Send的恢复
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueSocket, SelfSocket);
		DetourDetach(&(PVOID&)TrueConnect, SelfConnect);
		DetourDetach(&(PVOID&)TrueSend, SelfSend);
		//Sendto的恢复
		DetourDetach(&(PVOID&)TrueSendto, SelfSendto);
		//closeSocket的恢复
		DetourDetach(&(PVOID&)TrueClosesocket, SelfClosesocket);
		DetourTransactionCommit();
		fprintf(fp, "DLL DETACH\n");
		fclose(fp);
		delete rule_to_filter_instance;
	}
	return TRUE;
}
_declspec(dllexport) int go(int)
{
	return 0;
}