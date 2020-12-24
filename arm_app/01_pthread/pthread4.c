#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

static char g_buf[1000];

static sem_t g_sem;   /*����ȫ���ź���*/
pthread_mutex_t g_tmutex = PTHREAD_MUTEX_INITIALIZER;  /*���岢��ʼ��ȫ�ֻ�����*/
static void *my_pthread_fun (void *data)
{
	while(1)
	{
		/*�ȴ��ź���*/
		sem_wait(&g_sem);
		
		/*��ӡ*/
		pthread_mutex_lock(&g_tmutex);
		printf("recv:%s\n",g_buf);
		pthread_mutex_unlock(&g_tmutex);
	}
	return NULL;
}

int main(int argc, char **argv)
{	
	int ret;
	pthread_t tid;
	char buf[1000];
	/*��ʼ���ź���*/
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
		fgets(buf, 1000, stdin);
		pthread_mutex_lock(&g_tmutex);   /*���̻߳�ȡ������*/
		memcpy(g_buf, buf, 1000);        /*�޸�g_buf*/
		pthread_mutex_unlock(&g_tmutex); /*���߳��ͷŻ�����*/
		/*֪ͨ�����߳�*/
		sem_post(&g_sem);   /*�ͷ��ź���*/
	}
	
	return 0;
	
}


