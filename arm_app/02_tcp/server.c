
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#define SERVER_PORT 8888     //�����˿ں�
#define BACKLOG 10           //������ӵȴ���


/*socket
 *bind
 *listen
 *accept
 *send
 *recv
 */

int main(int argc, char **argv)
{
	int iSocketServer;   //�����socket���
	int iSocketClient;   //�ͻ���socket���
	
	/*sockaddr_in �� sockaddr ��Сһ����sockaddr_in��ip�Ͷ˿ڷֿ���*/
	struct sockaddr_in tSocketServerAddr;   //����˵�ַ�ṹ
	struct sockaddr_in tSocketClientAddr;   //�ͻ��˵�ַ�ṹ
	int iRet;
	int iAddrlen;     //��ַ����

	int iRecvLen;   //���յĳ���
	unsigned char ucRecvBuff[1000];   //���մ��buff

	int iClientNum = -1;    //�ͻ��˵�������

	signal(SIGCHLD, SIG_IGN);   //�����ӽ����˳��źţ�������ʬ����

	/*socket()��һ������ͨѶ�˿ڣ��ɹ�����һ�����������������󷵻�-1
	 *��һ����������ַ��:AF_INET��ʾipv4
	 *�ڶ����������������ͣ�TCPΪSOCK_STREAM��UDPΪSOCK_DGRAM
	 *����������������Э�飬ָ��Ϊ0��ϵͳ�Զ����ݴ���Э������
	 */
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if(iSocketServer == -1)
	{
		printf("socket error!\n");
		return -1;
	}

	/*����˵�ַ�ṹ������*/
	tSocketServerAddr.sin_family      = AF_INET;   //��ַ��
	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;   //ʹ�ñ��������ip��ַ
	tSocketServerAddr.sin_port        = htons(SERVER_PORT); //�˿ڣ�host to net �����ֽ���תΪ�����ֽ���
	memset(tSocketServerAddr.sin_zero, 0, 8);     //��0

	/*��socket���iSocketServer���ip���˿ڰ󶨣��ɹ�����0��ʧ�ܷ���-1
	 *ʹsocket����ͨѶ���ļ�������iSocketServer����tSocketServerAddr�������ĵ�ַ�Ͷ˿ں�
	 */
	iRet = bind(iSocketServer, (struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if(iRet == -1)
	{
		printf("bind erroe!\n");
		return -1;
	}

	/*����iSocketServer���ڼ���״̬���������BACKLOG�����ӵȴ�
	 *�ɹ�����0��ʧ�ܷ���-1
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
		/*һֱ�ȴ�������
		 *��������֮�󣬷���������accept��������
		 *�ɹ���ѿͻ��˵ĵ�ַ�˿ڴ����tSocketClientAddr���������ؿͻ���socket������
		 *ʧ�ܷ���-1
		 */
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, &iAddrlen);
		if(iSocketClient != -1)
		{
			++iClientNum;
			printf("Get connect from client %d: %s\n", iClientNum,inet_ntoa(tSocketClientAddr.sin_addr));
			/*�����ӽ������ڴ���������*/
			if(!fork())
			{
				/*�ӽ��̵�Դ��*/
				while(1)
				{
					/*���տͻ��˷��������ݲ���ʾ*/
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




















