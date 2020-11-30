
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* keytest on
  */
int main(int argc, char **argv)
{
	int fd;
	char key_val[4];
	int count;
	fd = open("/dev/button", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}
	
	while(1)
	{
		/*查询方式获取按键值*/
		read(fd, key_val, sizeof(key_val));
		//printf("%d %d %d %d\n", key_val[0],key_val[1],key_val[2],key_val[3]);
		if((key_val[0] == 0) || (key_val[1] == 0) || (key_val[2] == 0))
		{
			printf("%d times,val %d %d %d %d\n", ++count,key_val[0],key_val[1],key_val[2],key_val[3]);
		}
	}
	
	return 0;
}
