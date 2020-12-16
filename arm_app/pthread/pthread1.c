#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static void *my_pthread_fun (void *data)
{
	while(1)
	{
		sleep(1);
	}
}

int main(int argc, char **argv)
{	
	int ret;
	pthread_t tid;
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
		sleep(1);
	}
	
	return 0;
	
}


