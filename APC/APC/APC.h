#pragma once
#include "windivert.h"

//����������
#define filter_type_sniff WINDIVERT_FLAG_SNIFF
#define filter_type_drop WINDIVERT_FLAG_DROP
#define filter_type_modify WINDIVERT_FLAG_DEBUG

//����������
#define SNIFF_QUEUE_LEN 8192
#define SNIFF_QUEUE_TIME 5000
#define SNIFF_QUEUE_SIZE 4194304

#define MODIFY_QUEUE_LEN  20480
#define MODIFY_QUEUE_TIME 8000
#define MODIFY_QUEUE_SIZE 33554432


//�̸߳���
#define thread_num 1

//buffer��С
#define maxbuf 2048 
//ÿ�����ݰ����Ͳ�1500�ֽ�