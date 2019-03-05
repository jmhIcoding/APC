#pragma comment(lib,"WS2_32")
#pragma comment(lib,"detours")
#pragma comment(lib,"syelog")
#include <windows.h>
#include <detours.h>
#include <TlHelp32.h>
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
	char * chrome = (char *) "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
	char *ie = (char *)"C:\\Program Files\\internet explorer\\iexplore.exe";
	char *client = (char *)"C:\\Users\\jmh081701\\Documents\\APC\\APC\\x64\\Debug\\client.exe";
	char *netease = (char *) "C:\\Program Files (x86)\\Netease\\CloudMusic\\cloudmusic.exe";
	char * appFILE = netease;
	//远程注入DLL的版本
	//1. 获取进程id 
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(pe) };
	for (int ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe))
	{
		if (strcmp(pe.szExeFile, "psiphon3.exe") == 0)
		{
			printf("\t%s: %d\n", pe.szExeFile, pe.th32ProcessID);

			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, false, pe.th32ProcessID);
			int nlen = sizeof(WCHAR)*(wcslen(L"C:\\Users\\jmh081701\\Documents\\APC\\APC\\x64\\Debug\\APC.dll") + 1);
			LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, nlen, MEM_COMMIT, PAGE_READWRITE);
			if (INVALID_HANDLE_VALUE == pszLibFileRemote)
			{
				printf("分配失败\n");
			}
			DWORD  n = WriteProcessMemory(hProcess, pszLibFileRemote, L"C:\\Users\\jmh081701\\Documents\\APC\\APC\\x64\\Debug\\APC.dll", nlen, NULL);
			PTHREAD_START_ROUTINE threadFn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryW");
			HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, threadFn, pszLibFileRemote, 0, NULL);
			WaitForSingleObject(hThread, -1);
			CloseHandle(hThread);
			VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_FREE);
			printf("Inject well...\n");
		}
	//printf("\t%s: %d\n", pe.szExeFile, pe.th32ProcessID);
		
	}
	//system("pause");
	return 0;
}