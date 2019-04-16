# 引言
本人开发这个项目是为了学习cs架构的通信方式，同时学习一些并发/多线程模型，这个项目是学习的过程记录。
 
---

# client-server 各文件描述
- makefile 文件是用来编译当前文件夹的文件的
- client 是用来客户端程序文件夹  
- server 是服务端程序文件夹  
- op 是相关操作的文件夹，用于启停脚本

# 1 op
相关的client端 和 server 端的启停操作，以及一些监控脚本。

# 2 client
为了测试需要，client 分为单进程和多进程。其中单进程为client.c，多进程版本为client_multi，其中用到了线程池。
## 2.1 client.c
client.c 这是一个简单的模型，一次连接便断开
	client.c 是用于测试与server的连接
	从建立连接到结束连接主要有四个步骤。
		connect
		{send, receive}
		close
## 2.2 client_multi
这里主要是开启了一个线程池，封装了一个任务类，可以灵活设置任务量的大小和并发度。


# 3 server
这里是server 端的文件，基本的为server_naive，后面需要做并发于是用了server_multi。

## 3.1 server_select
这里是用select封装了一个并发度为1000的服务端，用到了IO复用等知识。

## 3.2 server_naive
在server_naive文件夹中，是一些简单的server模型，server[%d].c 是各种服务器，下面简要介绍一下：

### 3.2.1 循环服务器模型
	description:	
		一次只能服务一个客户端，在前面一个连接结束之后后者才能建立新的连接。
		主要包括5个步骤。
			listen
			accept
			{send, receive}
			close
	
	advantage:
		模型简单，适合单客户端
	disadvantage:
		不可以处理多客户端情况



### 3.2.2 统一accept, one request one process
	description:	
		主进程负责统一accept模块，在和每一个client建立连接后把数据处理交给另一个进程/线程处理。
		这样可以服务更多的client，模型如下
		主要包括5个步骤。
			listen
			accept
			fork()
			-main-	-child-
					{send, receive}
					close
			accept
			fork()
			-main-	-child-
					{send, receive} 
					close
			...
			close
	
	advantage:
		可以服务多个线程，灵活分配资源
	disadvantage:
		每次新建资源时间过长，上下文开销过大


### 3.2.3 统一accept, 预先分配线程池处理客户端
	description:
		首先主进程负责监听和接受请求。然后建立一个线程池，负责处理数据通信等业务模块。
		主进程负责统一accept模块，在和每一个client建立连接后把数据处理交给另一个进程/线程处理。

	主模块：
		create_thread_pool()
		listen
		accept
			alert thread
		accept,
			alert thread
		...
		close

	子进程：
		get connection,
		{send, receive},
		close
		

	advantage:
		可处理多客户端；预先定义好资源，分配简单。
	disadvantage:
		子进程个数不好评估，太少的话基本上退化成循环网络模型；	太多的话上下文切换是不低的开销。
		可以考虑动态调整的方式，根据系统当前负载和请求量调整子进程个数，Apache服务就是如此处理。


### 3.2.4 独立accept, 预先分配线程池处理全部流程
	主进程建立监听窗口，然后创建线程池，每一个线程池独立负责所有的工作。

	主模块：
		create_thread_pool()
		listen
			{choose_thread(),}
		close

	子进程：
		accept
		{send, receive,}
		close
		

	advantage:
		同2.3类似，少了accept的主线程瓶颈。
	disadvantage:
		所有的子线程都在竞争一个accept资源，会造成“惊群”现象，不过linux内核已经处理过这种case。



### 3.2.5 独立accept, 预先分配线程池处理全部流程
	此模式是在2.3的基础之上，将网络线程细分为处理连接和处理请求两类网络线程(TAF采用此框架)。

	主模块：
		create_accept_thread()
		create_handle_thread()
		create seats to flag which client has request
		listen
			each seat to check
		close

	accept 子进程：
		accept,
		choose one empty seat
			flag the seat
			set connected file
		...
		accept,
		choose one empty seat
			flag the seat
			set connected file
		
	handle 子进程：
		check all seats, for flaged seat
			{send, receive,}
			close connected file
				
	advantage:
		当今很多网络框架基本是按照这种模式来处理的，网络进程1监听客户端连接请求，把已完成的网络套接字给其它的网络进程处理，
		这些网络进程解析网络数据包，把数据放入消息队列(消息队列也可以起到过载保护的作用)中等待业务进程处理。
		业务线程处理完之后回包给网络线程，再由网络线程回给客户端。
		下面简单结合TAF服务端代码简析此模型：

Last edit at 20190417
