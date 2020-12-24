
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


/*socket
 *connect
 *send/recv
 */


int main(int argc, char **argv)
{
	int iSocketClient;
	struct sockaddr_in tSocketServerAddr;
	int iRet;

	int iSendLen;    //发送数据长度
	unsigned char ucSendBuff[1000];   //发送数据buff

	if(argc != 3)
	{
		printf("Usage:\n");
		printf("%s <server_ip> <port>\n", argv[0]);
	}

	/*socket()打开一个网络通讯端口，成功返回一个正数描述符，错误返回-1
	 *第一个参数，地址族:AF_INET表示ipv4
	 *第二个参数，传输类型，TCP为SOCK_STREAM，UDP为SOCK_DGRAM
	 *第三个参数，传输协议，指定为0，系统自动推演传输协议类型
	 */
	iSocketClient = socket(AF_INET, SOCK_STREAM, 0);

	/*设置服务器地址*/
	tSocketServerAddr.sin_family      = AF_INET;   //地址族
	//tSocketServerAddr.sin_addr.s_addr = inet_addr(argv[1]);   //这种方式返回值不清晰
	if(0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr))   
	{
		printf("invalid server_ip\n");
		return -1;
	}
	tSocketServerAddr.sin_port        = htons(atoi(argv[2])); //端口，host to net 主机字节序转为网络字节序
	memset(tSocketServerAddr.sin_zero, 0, 8);     //清0

	/*连接服务器
	 *成功返回0，失败返回-1
	 */
	iRet = connect(iSocketClient, (struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if(iRet == -1)
	{
		printf("connect error!\n");
		return -1;
	}
	while(1)
	{
		if(fgets(ucSendBuff, 999, stdin))
		{
			/*发送数据*/
			iSendLen = send(iSocketClient, ucSendBuff, strlen(ucSendBuff), 0);
			if(iSendLen <= 0)
			{
				close(iSocketClient);
				return -1;
			}
		}
	}
	
	return 0;
}















