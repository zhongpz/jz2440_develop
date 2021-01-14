
/* 参考 
 * drivers\mtd\nand\s3c2410.c
 * drivers\mtd\nand\at91_nand.c
 */


#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
  
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
  
#include <asm/io.h>
  
#include <asm/arch/regs-nand.h>
#include <asm/arch/nand.h>


static struct nand_chip *s3c_nand_chip;
static struct mtd_info *s3c_mtd;

struct s3c_nand_regs {
	unsigned long nfconf  ;
	unsigned long nfcont  ;
	unsigned long nfcmmd   ;
	unsigned long nfaddr  ;
	unsigned long nfdata  ;
	unsigned long nfeccd0 ;
	unsigned long nfeccd1 ;
	unsigned long nfeccd  ;
	unsigned long nfstat  ;
	unsigned long nfestat0;
	unsigned long nfestat1;
	unsigned long nfmecc0 ;
	unsigned long nfmecc1 ;
	unsigned long nfsecc  ;
	unsigned long nfsblk  ;
	unsigned long nfeblk  ;
};

static struct s3c_nand_regs *s3c_nand_regs;

static struct mtd_partition s3c_nand_parts[] = {
	[0] = {
        .name   = "bootloader",
        .size   = 0x00040000,     //分区大小
		.offset	= 0,   
	},
	[1] = {
        .name   = "params",
        .offset = MTDPART_OFS_APPEND,   //紧接上一个地址
        .size   = 0x00020000,
	},
	[2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_APPEND,
        .size   = 0x00200000,
	},
	[3] = {
        .name   = "root",
        .offset = MTDPART_OFS_APPEND,
        .size   = MTDPART_SIZ_FULL,    //剩下的全给root分区
	}
};


/***************************************
 * 函 数 名：s3c2440_cmd_ctrl
 * 函数功能：给nand flash 发送命令和地址
 * 输入参数：*mtd，dat，ctrl
 * 输出参数：无
 * 返 回 值：无
 ****************************************/
static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/* 发命令: NFCMMD=dat */
		s3c_nand_regs->nfcmmd = dat;
	}
	else
	{
		/* 发地址: NFADDR=dat */
		s3c_nand_regs->nfaddr = dat;
	}
}

/***************************************
 * 函 数 名：nand_select_chip
 * 函数功能：选中nand flash 芯片函数
 * 输入参数：*mtd，chipnr
 * 输出参数：无
 * 返 回 值：无
 ****************************************/
static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
		/* 取消选中：NFCONT[1]设为1 */
		s3c_nand_regs->nfcont |= (1 << 1);  
	}
	else
	{
		/* 选中：NFCONT[1]设为0 */
		s3c_nand_regs->nfcont &= ~(1 << 1);
	}
}

/***************************************
 * 函 数 名：s3c2440_dev_ready
 * 函数功能：判断读写是否完成
 * 输入参数：*mtd
 * 输出参数：无
 * 返 回 值：寄存器NFSTAT的bit[0]
 ****************************************/
static int s3c2440_dev_ready(struct mtd_info *mtd)
{
	return (s3c_nand_regs->nfstat & (1 << 0));
}



/* 入口函数 */
static int s3c_nand_init(void)
{
	struct clk *clk;
	
	/* 1.分配nand_chip结构 */
	s3c_nand_chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);

	s3c_nand_regs = ioremap(0x4e000000, sizeof(struct s3c_nand_regs));	
	
	/* 2.设置nand_chip结构 */
	s3c_nand_chip->select_chip = s3c2440_select_chip;
	s3c_nand_chip->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand_chip->IO_ADDR_R   = &s3c_nand_regs->nfdata;  //读函数中用到的地址
	s3c_nand_chip->IO_ADDR_W   = &s3c_nand_regs->nfdata;  //写函数中用到的地址
  	s3c_nand_chip->dev_ready   = s3c2440_dev_ready;
	s3c_nand_chip->ecc.mode    = NAND_ECC_SOFT;   //ECC校验

	/* 3.硬件相关的操作 */
	/* 使能nand flash 控制器的时钟 */
	clk = clk_get(NULL, "nand");   //获得时钟
	clk_enable(clk);       //使能时钟，设置CLKCON'bit[4]

	/* HCLK=100MHz
	 * TACLS:  发出CLE/ALE之后多长时间才发出nWE信号, 从NAND手册可知CLE/ALE与nWE可以同时发出,所以TACLS=0
	 * TWRPH0: nWE的脉冲宽度, HCLK x ( TWRPH0 + 1 ), 从NAND手册可知它要>=12ns, 所以TWRPH0>=1
	 * TWRPH1: nWE变为高电平后多长时间CLE/ALE才能变为低电平, 从NAND手册可知它要>=5ns, 所以TWRPH1>=0
	 */
#define TACLS    0
#define TWRPH0   1
#define TWRPH1   0
	s3c_nand_regs->nfconf = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);

	/* NFCONT: 
	 * BIT1-设为1, 取消片选 
	 * BIT0-设为1, 使能NAND FLASH控制器
	 */
	s3c_nand_regs->nfcont = (1<<1) | (1<<0);

	/* 4.使用:nand_scan */
	/* 4.1 nand_scan函数要用到mtd_info结构，所以分配一个 */
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	/* 4.2将nand_chip结构和mtd_info结构联系起来 */
	s3c_mtd->priv = s3c_nand_chip;  
	s3c_mtd->owner = THIS_MODULE;
	/* nand_scan根据nand_chip的底层操作函数识别NAND FLASH，设置mtd_info结构（读写擦除） */
	nand_scan(s3c_mtd, 1);  //参2表示芯片个数

	
	/* 5.增加mtd分区：add_mtd_partitons */
	//add_mtd_device();  //如果直接用一个分区的话
	add_mtd_partitions(s3c_mtd, s3c_nand_parts, 4);  //构造分区

	
	return 0;
}

/* 出口函数 */
static void s3c_nand_exit(void)
{
	del_mtd_partitions(s3c_mtd);
	kfree(s3c_mtd);
	iounmap(s3c_nand_regs);
	kfree(s3c_nand_chip);
}


module_init(s3c_nand_init);
module_exit(s3c_nand_exit);
MODULE_LICENSE("GPL");












