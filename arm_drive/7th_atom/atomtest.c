
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>


/* seventhdrvtest 
  */

int fd;

/*信号处理函数*/
void my_signal_fun(int signum	)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("key_val: 0x%x\n", key_val);
}

int main(int argc, char **argv)
{
	int Oflags;
	int ret;
	unsigned char key_val;
	//signal(SIGIO, my_signal_fun);
	fd = open("/dev/button", O_RDWR | O_NONBLOCK);
	if (fd < 0)
	{
		printf("can't open!\n");
		return -1;	
	}

	/*告诉内核pid*/
	//fcntl(fd, F_SETOWN, getpid());

	/*获取flag*/
	//Oflags = fcntl(fd, F_GETFL); 

	/*设置flag*/
	//fcntl(fd, F_SETFL, Oflags | FASYNC);
	
	while (1)
	{	
		ret = read(fd, &key_val, 1);
		printf("key_val: 0x%x, ret = %d\n", key_val, ret);
		sleep(5);
	}
	
	return 0;
}

