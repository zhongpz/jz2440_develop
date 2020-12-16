#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

static char g_buf[1000];

static sem_t g_sem;
pthread_mutex_t g_tmutex = PTHREAD_MUTEX_INITIALIZER;
static void *my_pthread_fun (void *data)
{
	while(1)
	{
		/*等待信号量*/
		sem_wait(&g_sem);
		
		/*打印*/
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
	sem_init(&g_sem, 0, 0);
	/*1.创建一个接收线程*/
	ret = pthread_create(&tid, NULL, my_pthread_fun, NULL);
	if(ret != 0)
	{
		printf("thread_create err\n");
		return -1;
	}

	/*2.主线程读取标准输入发给接收线程*/
	while(1)
	{	
		fgets(buf, 1000, stdin);
		pthread_mutex_lock(&g_tmutex);
		memcpy(g_buf, buf, 1000);
		pthread_mutex_unlock(&g_tmutex);
		/*通知接收线程*/
		sem_post(&g_sem);
	}
	
	return 0;
	
}


