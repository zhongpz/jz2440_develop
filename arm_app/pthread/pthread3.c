#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

static char g_buf[1000];

static sem_t g_sem;

static void *my_pthread_fun (void *data)
{
	while(1)
	{
		/*�ȴ��ź���*/
		sem_wait(&g_sem);
		
		/*��ӡ*/
		printf("recv:%s\n",g_buf);
	}
	return NULL;
}

int main(int argc, char **argv)
{	
	int ret;
	pthread_t tid;

	sem_init(&g_sem, 0, 0);
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
		sem_post(&g_sem);
	}
	
	return 0;
	
}


