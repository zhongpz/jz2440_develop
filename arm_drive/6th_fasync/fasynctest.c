
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>


/* sixthdrvtest 
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
	signal(SIGIO, my_signal_fun);
	fd = open("/dev/button", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	/*告诉内核pid*/
	fcntl(fd, F_SETOWN, getpid());

	/*获取flag*/
	Oflags = fcntl(fd, F_GETFL); 

	/*设置flag*/
	fcntl(fd, F_SETFL, Oflags | FASYNC);
	
	while (1)
	{	
		sleep(1000);
	}
	
	return 0;
}

