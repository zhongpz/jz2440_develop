
/* 参考：
 * drivers\block\xd.c
 * drivers\block\z2ram.c
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>
 
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

#define RAMBLOCK_SIZE (1024*1024)   //块设备容量1M
static unsigned char *ramblock_buf;  

static int major;

static struct gendisk *ramblock_disk;
static struct request_queue_t *ramblock_queue;

static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
};
static DEFINE_SPINLOCK(ramblock_lock); //自旋锁


/***************************************
 * 函 数 名：do_ramblock_request
 * 函数功能：队列处理函数
 * 输入参数：*q,队列
 * 输出参数：无
 * 返 回 值：无
 ***************************************/
static void do_ramblock_request(request_queue_t * q)
{
	static int r_cnt = 0;
	static int w_cnt = 0;
	static struct request *req;
	static int cnt = 0;
	//printk("do_ramblock_request %d\n", ++cnt);

	/* while中做真正硬件相关的操作，内存模拟磁盘（用memcpy实现内存读写） */
	while((req = elv_next_request(q)) != NULL)
	{
		/* 数据传输三要素：源，目的，长度 */
		/* 源 或 目的 */
		unsigned long offset = req->sector * 512;  //下一个要处理的扇区（一个扇区512字节）
		
		/* 目的 或 源 */
		//req->buffer
		
		/* 长度 */
		unsigned long len = req->current_nr_sectors * 512;  //待处理的扇区个数

		/* 读写实现 */
		if(rq_data_dir(req) == READ) //若是读
		{
			/* memcpy(目的， 源， 长度) */
			printk("do_rambloc_request read %d\n", ++r_cnt);
			memcpy(req->buffer, ramblock_buf + offset, len);
		}
		else  //写则到过来
		{
			
			printk("do_rambloc_request write %d\n", ++w_cnt);
			memcpy(ramblock_buf + offset, req->buffer, len);
		}
		
		end_request(req, 1); //1成功，0失败
	}
}


/* 入口函数 */ 
static int ramblock_init(void)
{
	/* 1.分配一个gendisk结构体 */
	ramblock_disk = alloc_disk(16); //16表示可以创建：“1个主磁盘” + “15个分区”
	
	/* 2.设置 */
	/* 2.1分配/设置队列：提供读写能力 */
	ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock); //分配队列
	ramblock_disk->queue = ramblock_queue; //设置队列	
	
	/* 2.2设置其他属性：比如容量等等 */
	major = register_blkdev(0, "ramblock"); /* 返回一个主设备，cat /proc/devices */

	ramblock_disk->major	   = major;     //主设备号
	ramblock_disk->first_minor = 0;         //第一个此设备号
	sprintf(ramblock_disk->disk_name, "ramblock");  //块设备名字
	ramblock_disk->fops 	   = &ramblock_fops;    //操作函数结构体
	set_capacity(ramblock_disk, RAMBLOCK_SIZE / 512);   //容量，以扇区为单位，一个扇区512字节

	/* 3.硬件相关的操作 */
	ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);  //分配一块1M的内存
	
	/* 4.注册 */
	add_disk(ramblock_disk);
	
	return 0;
}

/* 出口函数 */
static void ramblock_exit(void)
{
	unregister_blkdev(major, "ramblock");   //卸载register_blkdev的块设备
	del_gendisk(ramblock_disk);             //清除alloc_disk的结构
	put_disk(ramblock_disk);                //释放set_capacity的空间
	blk_cleanup_queue(ramblock_queue);      //清除blk_init_queue的队列

	kfree(ramblock_buf);
}


module_init(ramblock_init);
module_exit(ramblock_exit);

MODULE_LICENSE("GPL");






