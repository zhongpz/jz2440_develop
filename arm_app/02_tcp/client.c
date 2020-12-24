
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

	int iSendLen;    //�������ݳ���
	unsigned char ucSendBuff[1000];   //��������buff

	if(argc != 3)
	{
		printf("Usage:\n");
		printf("%s <server_ip> <port>\n", argv[0]);
	}

	/*socket()��һ������ͨѶ�˿ڣ��ɹ�����һ�����������������󷵻�-1
	 *��һ����������ַ��:AF_INET��ʾipv4
	 *�ڶ����������������ͣ�TCPΪSOCK_STREAM��UDPΪSOCK_DGRAM
	 *����������������Э�飬ָ��Ϊ0��ϵͳ�Զ����ݴ���Э������
	 */
	iSocketClient = socket(AF_INET, SOCK_STREAM, 0);

	/*���÷�������ַ*/
	tSocketServerAddr.sin_family      = AF_INET;   //��ַ��
	//tSocketServerAddr.sin_addr.s_addr = inet_addr(argv[1]);   //���ַ�ʽ����ֵ������
	if(0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr))   
	{
		printf("invalid server_ip\n");
		return -1;
	}
	tSocketServerAddr.sin_port        = htons(atoi(argv[2])); //�˿ڣ�host to net �����ֽ���תΪ�����ֽ���
	memset(tSocketServerAddr.sin_zero, 0, 8);     //��0

	/*���ӷ�����
	 *�ɹ�����0��ʧ�ܷ���-1
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
			/*��������*/
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















