#pragma once
#include "APC.h"
#include <windivert.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <vector>
using namespace std;
/*
		������ˡ��޸���������WinDivertʵ��
*/

class netfilter
{
public:
	void * winDivertHandle;
	vector<HANDLE> thread_id;
public:
	netfilter();
	netfilter(char * filter, char filter_type = filter_type_modify);
	~netfilter();
	void close_filter();
};

