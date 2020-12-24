
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#define SERVER_PORT 8888     //本机端口号
#define BACKLOG 10           //最大连接等待数


/*socket
 *bind
 *listen
 *accept
 *send
 *recv
 */

int main(int argc, char **argv)
{
	int iSocketServer;   //服务端socket句柄
	int iSocketClient;   //客户端socket句柄
	
	/*sockaddr_in 和 sockaddr 大小一样，sockaddr_in把ip和端口分开了*/
	struct sockaddr_in tSocketServerAddr;   //服务端地址结构
	struct sockaddr_in tSocketClientAddr;   //客户端地址结构
	int iRet;
	int iAddrlen;     //地址长度

	int iRecvLen;   //接收的长度
	unsigned char ucRecvBuff[1000];   //接收存放buff

	int iClientNum = -1;    //客户端的连接数

	signal(SIGCHLD, SIG_IGN);   //忽略子进程退出信号，反正僵尸进程

	/*socket()打开一个网络通讯端口，成功返回一个正数描述符，错误返回-1
	 *第一个参数，地址族:AF_INET表示ipv4
	 *第二个参数，传输类型，TCP为SOCK_STREAM，UDP为SOCK_DGRAM
	 *第三个参数，传输协议，指定为0，系统自动推演传输协议类型
	 */
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if(iSocketServer == -1)
	{
		printf("socket error!\n");
		return -1;
	}

	/*服务端地址结构的设置*/
	tSocketServerAddr.sin_family      = AF_INET;   //地址族
	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;   //使用本机任意的ip地址
	tSocketServerAddr.sin_port        = htons(SERVER_PORT); //端口，host to net 主机字节序转为网络字节序
	memset(tSocketServerAddr.sin_zero, 0, 8);     //清0

	/*把socket句柄iSocketServer绑和ip、端口绑定，成功返回0，失败返回-1
	 *使socket网络通讯的文件描述符iSocketServer监听tSocketServerAddr所描述的地址和端口号
	 */
	iRet = bind(iSocketServer, (struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if(iRet == -1)
	{
		printf("bind erroe!\n");
		return -1;
	}

	/*声明iSocketServer处于监听状态，最多允许BACKLOG个连接等待
	 *成功返回0，失败返回-1
	 */
	iRet = listen(iSocketServer, BACKLOG);
	if(iRet == -1)
	{
		printf("error bind!\n");
		return -1;
	}

	while(1)
	{
		iAddrlen = sizeof(struct sockaddr);
		/*一直等待被连接
		 *三次握手之后，服务器调用accept接受连接
		 *成功则把客户端的地址端口存放在tSocketClientAddr参数，返回客户端socket描述符
		 *失败返回-1
		 */
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, &iAddrlen);
		if(iSocketClient != -1)
		{
			++iClientNum;
			printf("Get connect from client %d: %s\n", iClientNum,inet_ntoa(tSocketClientAddr.sin_addr));
			/*创建子进程用于处理多个连接*/
			if(!fork())
			{
				/*子进程的源码*/
				while(1)
				{
					/*接收客户端发来的数据并显示*/
					iRecvLen = recv(iSocketClient, ucRecvBuff, 999, 0);
					if(iRecvLen <= 0)
					{
						printf("child exit\n");
						close(iSocketClient);
						return -1;
					}
					else
					{
						ucRecvBuff[iRecvLen] = '\0';
						printf("Get msg from client %d: %s\n",iClientNum, ucRecvBuff);
					}
				}
			}
		}
	}
	close(iSocketServer);
	return 0;
}




















