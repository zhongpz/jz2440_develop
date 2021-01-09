
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


/* keytest on
  */
int main(int argc, char **argv)
{
	int fd;
	int ret;
	char val = 'a';
	fd = open("/dev/uart", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}
	write(fd, &val, 1);
	return 0;
}

