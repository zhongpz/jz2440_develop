
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* seconddrvtest on
  * seconddrvtest off
  */
int main(int argc, char **argv)
{
	int fd;
	int val = 1;
	if (argc != 2)
	{
		printf("Usage :\n");
		printf("%s <on|off>\n", argv[0]);
		return 0;
	}
	fd = open("/dev/led", O_RDWR);
		if (fd < 0)
		{
			printf("can't open!\n");
		}

	if (strcmp(argv[1], "on") == 0)
	{
		val  = 1;
	}
	else
	{
		val = 0;
	}
	
	write(fd, &val, 4);
	return 0;
}
