#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/proc_fs.h>

#define MYLOG_BUF_LEN 1024

struct proc_dir_entry *myentry;
static DECLARE_WAIT_QUEUE_HEAD(mymsg_waitq);


static char mylog_buff[MYLOG_BUF_LEN];
static char tmp_buff[MYLOG_BUF_LEN];

static int mylog_r = 0;   //环形缓冲区读标志
static int mylog_w = 0;   //环形缓冲区写标志

/* 环形缓冲区 */
static int is_mylog_empty(void)
{
	return(mylog_r == mylog_w);
}

static int is_mylog_full(void)
{
	return((mylog_w + 1) % MYLOG_BUF_LEN == mylog_r);
}

static void mylog_putc(char c)
{
	if(is_mylog_full())
	{
		/* 如果满了，丢弃一个数据 */
		mylog_r = (mylog_r + 1) % MYLOG_BUF_LEN;
	}

	mylog_buff[mylog_w] = c;
	mylog_w = (mylog_w + 1) % MYLOG_BUF_LEN;
	
	/* 唤醒等待数据的进程 */	
    wake_up_interruptible(&mymsg_waitq);   /* 唤醒休眠的进程 */	
}

static int mylog_getc(char *p)
{
	if(is_mylog_empty())
	{
		return 0;
	}
	*p = mylog_buff[mylog_r];
	mylog_r = (mylog_r + 1) % MYLOG_BUF_LEN;
	return 1;
}

/* 模仿sprintf函数，写自己的myprintk函数 */
int myprintk(const char *fmt, ...)
{
	va_list args;
	int i;
	int j;

	va_start(args, fmt);
	i=vsnprintf(tmp_buff, INT_MAX, fmt, args);
	va_end(args);

	/* tmp_buff中的内容写到mylog_buff */
	for(j = 0; j < i; j++)
	{
		mylog_putc(tmp_buff[j]);
	}
	
	return i;
}

/* 读函数 */
static ssize_t mymsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	int err;
	int i = 0;
	char c;
	/* 把mylog_buff的数据copy_to_user,return */
	if ((file->f_flags & O_NONBLOCK) && is_mylog_empty()) //非阻塞，且无数据
		return -EAGAIN;  //立即返回
		
 	err = wait_event_interruptible(mymsg_waitq, !is_mylog_empty());  //休眠等待

	/* copy_to_user */
	while (!err && (mylog_getc(&c)) && i < count) {
		err = __put_user(c, buf);
		buf++;
		i++;
	}
	if(!err)
		err = i;

	return err;	
}


const struct file_operations proc_mykmsg_operations = {
	.read = mymsg_read,
};


/* 入口函数 */
static int mymsg_init(void)
{
	myentry = create_proc_entry("mymsg", S_IRUSR, &proc_root);   //创建条目
	if (myentry)
		myentry->proc_fops = &proc_mykmsg_operations;   //file_operation结构
	return 0;
}

/* 出口函数 */
static void mymsg_exit(void)
{
	remove_proc_entry("mymsg", &proc_root); //去除条目
}

module_init(mymsg_init);
module_exit(mymsg_exit);
EXPORT_SYMBOL(myprintk);

MODULE_LICENSE("GPL");




