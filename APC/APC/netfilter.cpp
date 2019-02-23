#include "netfilter.h"


DWORD sniff_routine(void * handle)
{
	unsigned char buffer[maxbuf] = { 0 };
	unsigned int packet_len;
	WINDIVERT_ADDRESS pkt_addr;
	handle = (HANDLE)handle;
	while (true)
	{
		;
	}
}
DWORD modify_routine(LPVOID handle)
{
	unsigned char buffer[maxbuf] = { 0 };
	unsigned int packet_len;
	WINDIVERT_ADDRESS pkt_addr;
	handle = (HANDLE)handle;
	while (true)
	{
		WinDivertRecv(handle, buffer, maxbuf, &pkt_addr, &packet_len);//��������
		fprintf(stdout, "recv data...%d bytes.\n", packet_len);
		//�޸�����
		pkt_addr.Impostor = 1;

		buffer[4] = 0x01;//ipid�ֶ�
		buffer[5] = 0xEE;
		WinDivertHelperCalcChecksums(buffer, packet_len, &pkt_addr, WINDIVERT_HELPER_NO_ICMP_CHECKSUM | WINDIVERT_HELPER_NO_TCP_CHECKSUM | WINDIVERT_HELPER_NO_UDP_CHECKSUM | WINDIVERT_HELPER_NO_ICMPV6_CHECKSUM);

		WinDivertSend(handle, buffer, packet_len, &pkt_addr, NULL);//��������
		fprintf(stdout, "send data...%d bytes.\n", packet_len);
	}
}

netfilter::netfilter()
{
}
/*
	netfilter(char* filter,char filter_type)	����һ��������˶���
	������
		filter:���˹���,�� https://reqrypt.org/windivert-doc.html ��7����
		filter_type:���������ͣ��������֣�
							filter_type_sniff ����
							fitler_type_drop  ���������ѷ��Ϲ���İ�����
							filter_type_modify	�޸İ�,Ȼ�����·���
							
*/
netfilter::netfilter(char * filter, char filter_type)
{
	this->winDivertHandle = WinDivertOpen(filter,WINDIVERT_LAYER_NETWORK,-1000,filter_type);
	LPTHREAD_START_ROUTINE routine = (LPTHREAD_START_ROUTINE)NULL;
	if (winDivertHandle== INVALID_HANDLE_VALUE)
	{
		fprintf(stderr,"failed to open the WinDivert device (%d)", GetLastError());
		return;
	}
	switch (filter_type)
	{
	
	case filter_type_drop:
		break;
	case filter_type_sniff:
		WinDivertSetParam(this->winDivertHandle, WINDIVERT_PARAM_QUEUE_LEN, SNIFF_QUEUE_LEN);
		WinDivertSetParam(this->winDivertHandle, WINDIVERT_PARAM_QUEUE_TIME, SNIFF_QUEUE_TIME);
		WinDivertSetParam(this->winDivertHandle, WINDIVERT_PARAM_QUEUE_SIZE, SNIFF_QUEUE_SIZE);
		//routine = sniff_routine;
		break;
	default:
		WinDivertSetParam(this->winDivertHandle, WINDIVERT_PARAM_QUEUE_LEN, MODIFY_QUEUE_LEN);
		WinDivertSetParam(this->winDivertHandle, WINDIVERT_PARAM_QUEUE_TIME, MODIFY_QUEUE_TIME);
		WinDivertSetParam(this->winDivertHandle, WINDIVERT_PARAM_QUEUE_SIZE, MODIFY_QUEUE_SIZE);
		routine = (LPTHREAD_START_ROUTINE) modify_routine;
		break;
	}
	fprintf(stdout, "filter open well.\n");
	for (int i = 0; filter_type!=filter_type_drop &&  i < thread_num; i++)
	{
		HANDLE thread_handle = CreateThread(NULL, 1, routine, this->winDivertHandle, 0, NULL);
		thread_id.push_back(thread_handle);
	}
	//modify_routine(winDivertHandle);
}

netfilter::~netfilter()
{
	close_filter();
}
void netfilter::close_filter()
{
	if (winDivertHandle!=INVALID_HANDLE_VALUE)
	{
		for (int i = 0; i < thread_id.size(); i++)
		{
			if (thread_id[i] != INVALID_HANDLE_VALUE)
			{
				TerminateThread(thread_id[i], 0);
				CloseHandle(thread_id[i]);//�ر��߳�
			}
		}
		WinDivertClose(this->winDivertHandle);
	}
}