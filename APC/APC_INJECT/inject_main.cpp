#pragma comment(lib,"WS2_32")
#pragma comment(lib,"detours")
#pragma comment(lib,"syelog")
#include <windows.h>
#include <detours.h>
#include <stdio.h>
#include <string.h>
int main()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	char * DLLFILE = (char*)"C:\\Users\\jmh081701\\Documents\\APC\\APC\\x64\\Debug\\APC.dll";
	char * appFILE =(char *) "C:\\Program Files\\internet explorer\\iexplore.exe";
	if (TRUE == DetourCreateProcessWithDll(NULL,appFILE, 
		NULL, NULL, FALSE, 
		NORMAL_PRIORITY_CLASS , NULL, NULL, &si, &pi, DLLFILE, NULL)
		)
	{
		fprintf(stdout, "Inject %s well.\n", appFILE);
	}
	else
	{
		fprintf(stderr, "Inject %s error.\n", appFILE);
		fprintf(stderr, "Error ID:%d\n", GetLastError());
	}
	while (true)
	{
		;
	}
	return 0;
}