#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

static char g_buf[1000];

pthread_mutex_t g_tmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_tconvar = PTHREAD_COND_INITIALIZER;
static void *my_pthread_fun (void *data)
{
	while(1)
	{
		/*�ȴ���������*/
		 pthread_mutex_lock(&g_tmutex);
		 pthread_cond_wait(&g_tconvar, &g_tmutex);
		
		/*��ӡ*/
		
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
		pthread_mutex_lock(&g_tmutex);
		memcpy(g_buf, buf, 1000);
		pthread_cond_signal(&g_tconvar);/*֪ͨ�����߳�*/
		pthread_mutex_unlock(&g_tmutex);
		
	}
	
	return 0;
	
}


