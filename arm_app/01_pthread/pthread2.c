#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


static char g_buf[1000];
static int g_hasdata = 0;
static void *my_pthread_fun (void *data)
{
	while(1)
	{
		/*�ȴ�֪ͨ*/
		while(g_hasdata == 0);
		
		/*��ӡ*/
		printf("recv:%s\n",g_buf);
		g_hasdata = 0;
	}
	return NULL;
}

int main(int argc, char **argv)
{	
	int ret;
	pthread_t tid;
	/*1.����һ�������߳�*/
	ret = pthread_create(&tid, NULL, my_pthread_fun, NULL);
	if(ret != 0)
	{
		printf("thread_create err\n");
		return -1;
	}

	/*2.���̶߳�ȡ��׼���뷢�������߳�*/
	while(1)
	{
		fgets(g_buf, 1000, stdin);
		/*֪ͨ�����߳�*/
		g_hasdata = 1;
	}
	
	return 0;
	
}


