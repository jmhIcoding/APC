#include "netfilter.h"
#pragma comment(lib,"WS2_32")
#pragma comment(lib,"detours")
#pragma comment(lib,"syelog")
#pragma comment(lib,"WinDivert")
#include <detours.h>
#include "util.h"
/*
int main()
{
	struct sockaddr_in localaddr;
	int localaddrLen;
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(socketVersion, &wsaData);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_addr.S_un.S_addr = inet_addr("192.168.0.101");
	serAddr.sin_port = htons(9999);
	connect(s, (sockaddr*)&serAddr, sizeof(serAddr));

	int ret = getsockname(s, (struct sockaddr *)&localaddr, &localaddrLen);
	printf("main sin family:%d\n", localaddr.sin_family);

	filter_rule rule;
	get_sock_local_info(s, &rule);
	char buf[32];
	rule.tostring(buf, 32);
	WSACleanup();
	fprintf(stderr, buf);
	netfilter nf(buf,filter_type_modify);
	Sleep(10000);
	fprintf(stdout, "bye-bye\n");
	return 0;
}
*/


static LONG dwSlept = 0;

// Target pointer for the uninstrumented Sleep API.
//
static VOID(WINAPI * TrueSleep)(DWORD dwMilliseconds) = Sleep;

// Detour function that replaces the Sleep API.
//
VOID WINAPI TimedSleep(DWORD dwMilliseconds)
{
	// Save the before and after times around calling the Sleep API.
	DWORD dwBeg = GetTickCount();
	TrueSleep(dwMilliseconds);
	DWORD dwEnd = GetTickCount();

	InterlockedExchangeAdd(&dwSlept, dwEnd - dwBeg);
}

// DllMain function attaches and detaches the TimedSleep detour to the
// Sleep target function.  The Sleep target function is referred to
// through the TrueSleep target pointer.
//
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (dwReason == DLL_PROCESS_ATTACH) {
		DetourRestoreAfterWith();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueSleep, TimedSleep);
		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueSleep, TimedSleep);
		DetourTransactionCommit();
	}
	return TRUE;
}