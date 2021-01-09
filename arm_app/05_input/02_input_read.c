
#include <linux/input.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

/* get_input_info
 * 由命令行传入要打开的设备文件
 * 打开输入设备后，获取设备中的信息
 */

int main(int argc, char **argv)
{
	int fd;
	int error;
	int i, bit;
	unsigned char byte;
	
	struct input_id id;    //用于存放获得的设备id信息
	unsigned int evbit[2]; //用于存放获得的evbit信息
	int len;

	struct input_event event;  //read从内核驱动读回的数据结构

	char *ev_names[] = {
		"EV_SYN",
		"EV_KEY",	
		"EV_REL",	
		"EV_ABS",	
		"EV_MSC",	
		"EV_SW"	,	
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"NULL" ,
		"EV_LED",	
		"EV_SND",	
		"NULL" ,
		"EV_REP",	
		"EV_FF"	,	
		"EV_PWR",
	};
	
	/* 使用提示 */
	if(argc == 1)
	{
		printf("Usage: %s <dev> [noblock]\n", argv[0]);
		return -1;
	}
	
	/* 打开设备 */
	if(argc == 3 && !strcmp(argv[2], "noblock"))
	{
		fd = open(argv[1], O_RDWR | O_NONBLOCK); //非阻塞方式，可读可写
	}
	else
	{
		fd = open(argv[1], O_RDWR); //阻塞方式，可读可写
	}
	if(fd < 0)
	{
		printf("open %s error!\n", argv[1]);
		return -1;
	}

	/* ioctl获取设备信息 */
	/* 读取id信息 */
	error = ioctl(fd, EVIOCGID, &id);
	if(error == 0)
	{
		printf("bustype = 0x%x\n", id.bustype);
		printf("vendor  = 0x%x\n", id.vendor );
		printf("product = 0x%x\n", id.product);
		printf("version = 0x%x\n", id.version);	
	}

	/* 读取事件类型 */
	len = ioctl(fd, EVIOCGBIT(0, sizeof(evbit)),&evbit);
	
	//printf("len = %d\n",len);
	//printf("%d, %d\n", evbit[0], evbit[1]);
	
	if(len > 0 && len <=sizeof(evbit))
	{
		printf("support ev type: ");
		for(i = 0; i < len/2; i++)
		{
			byte = ((unsigned char *)evbit)[i];
			for(bit = 0; bit < 8; bit++)
			{
				if(byte & (1 << bit))
				{
					printf("%s ", ev_names[i*8 + bit]);
				}
			}
		}
		printf("\n");
	}

	while(1)
	{	
		len = read(fd, &event, sizeof(event));
		if(len == sizeof(event))
		{
			printf("get event: type = 0x%x, code = 0x%x, value = 0x%x\n", event.type, event.code, event.value);
		}
		else
		{
			printf("read error!\n");
		}
	}
	
	return 0;
}























