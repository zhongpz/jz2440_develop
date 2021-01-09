
#include <linux/input.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

int fd;
struct input_event event;  //read���ں��������ص����ݽṹ

/* get_input_info
 * �������д���Ҫ�򿪵��豸�ļ�
 * �������豸�󣬻�ȡ�豸�е���Ϣ
 */


/************************************************************
 * ��������: my_sig_fun
 * ��������: �źŴ�����
 * �������: sig �յ����ź�ֵ�\
 * �������: 
 * �� �� ֵ: 
 * �޸�����      �汾��      �޸���       �޸�����
 *------------------------------------------------------------
 *
 *************************************************************/
void my_sig_fun(int sig)
{
	while(read(fd, &event, sizeof(event)) == sizeof(event))
	{
		printf("get event: type = 0x%x, code = 0x%x, value = 0x%x\n", event.type, event.code, event.value);
	}
}


int main(int argc, char **argv)
{
	int error;
	int i, bit;
	unsigned char byte;
	
	struct input_id id;    //���ڴ�Ż�õ��豸id��Ϣ
	unsigned int evbit[2]; //���ڴ�Ż�õ�evbit��Ϣ
	int len;
	int ret;
	int flags;
	int count = 0;

	#if 0
	struct pollfd fds[1];
	nfds_t nfds = 1;
	#endif
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
	
	/* ʹ����ʾ */
	if(argc != 2)
	{
		printf("Usage: %s <dev>\n", argv[0]);
		return -1;
	}

	/* ע�ᴦ���� */
	signal(SIGIO, my_sig_fun);
	
	/* ���豸 */
	fd = open(argv[1], O_RDWR | O_NONBLOCK); //������ʽ���ɶ���д
	if(fd < 0)
	{
		printf("open %s error!\n", argv[1]);
		return -1;
	}

	/* ��APP���̺Ÿ����������� */
	fcntl(fd, F_SETOWN, getpid());
	
	/* ʹ���첽֪ͨ */
	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | FASYNC);
	
	/* ioctl��ȡ�豸��Ϣ */
	/* ��ȡid��Ϣ */
	error = ioctl(fd, EVIOCGID, &id);
	if(error == 0)
	{
		printf("bustype = 0x%x\n", id.bustype);
		printf("vendor  = 0x%x\n", id.vendor );
		printf("product = 0x%x\n", id.product);
		printf("version = 0x%x\n", id.version);	
	}

	/* ��ȡ�¼����� */
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
		#if 0
		fds[0].fd = fd;
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		ret = poll(fds, nfds, 5000);
		if(ret > 0)
		{
			if(fds[0].revents == POLLIN)
			{
				while(read(fd, &event, sizeof(event)) == sizeof(event))
				{
					printf("get event: type = 0x%x, code = 0x%x, value = 0x%x\n", event.type, event.code, event.value);
				}
			}
		}
		else if(ret == 0)
		{
			printf("time out!\n");
		}
		else
		{
			printf("poll error\n");
		}
		#endif
		printf("main loop count = %d\n", count++);	
		sleep(2);
		
	}
	
	return 0;
}























