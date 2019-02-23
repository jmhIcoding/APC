
	1.通过getsockname,获取特定socket_fd对应的端口、IP信息;这个信息是本地的信息
		目的是获取本侧的信息!(udp/tcp,src_port)
	2.通过detour函数,DLL注入,hook住主要的socket函数
		tcp: 
			send()函数,里面可以获取socket的值
		udp:
			sendto()函数,里面有socket的值
		
		做好过滤功能。。。。。
		
		closesocket() :关闭socket。释放过滤对象