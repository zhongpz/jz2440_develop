#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

static char g_buf[1000];

static sem_t g_sem;    /*定义信号量*/

static void *my_pthread_fun (void *data)
{
	while(1)
	{
		/*等待信号量，会进入休眠，降低cpu资源*/
		sem_wait(&g_sem);
		
		/*打印*/
		printf("recv:%s\n",g_buf);
	}
	return NULL;
}

int main(int argc, char **argv)
{	
	int ret;
	pthread_t tid;

	sem_init(&g_sem, 0, 0);   /*初始化信号量，第二个参数为0表示信号量在线程间共享，非0表示进程间共享，最后是信号量的值*/
	
	ret = pthread_create(&tid, NULL, my_pthread_fun, NULL);
	if(ret != 0)
	{
		printf("thread_create err\n");
		return -1;
	}

	/*2.主线程读取标准输入发给接收线程*/
	while(1)
	{
		fgets(g_buf, 1000, stdin);
		/*通知接收线程*/
		sem_post(&g_sem);   //释放信号量
	}
	
	return 0;
	
}


