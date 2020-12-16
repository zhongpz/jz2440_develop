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
		sleep(1);
	}
	
	return 0;
	
}


