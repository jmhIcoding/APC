
	1.ͨ��getsockname,��ȡ�ض�socket_fd��Ӧ�Ķ˿ڡ�IP��Ϣ;�����Ϣ�Ǳ��ص���Ϣ
		Ŀ���ǻ�ȡ�������Ϣ!(udp/tcp,src_port)
	2.ͨ��detour����,DLLע��,hookס��Ҫ��socket����
		tcp: 
			send()����,������Ի�ȡsocket��ֵ
		udp:
			sendto()����,������socket��ֵ
		
		���ù��˹��ܡ���������
		
		closesocket() :�ر�socket���ͷŹ��˶���