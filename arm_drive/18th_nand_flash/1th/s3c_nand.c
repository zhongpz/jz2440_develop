
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


struct nand_chip *s3c_nand_chip;
struct mtd_info *s3c_mtd;


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
	}
	else
	{
		/* 发地址: NFADDR=dat */
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
		/* 取消选中：NFCONT[1]设为0 */
	}
	else
	{
		/* 选中：NFCONT[1]设为1 */
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
	return "NFSTAT的bit[0]";
}



/* 入口函数 */
static int s3c_nand_init(void)
{
	/* 1.分配nand_chip结构 */
	s3c_nand_chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);

	
	/* 2.设置nand_chip结构 */
	s3c_nand_chip->select_chip = s3c2440_select_chip;
	s3c_nand_chip->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand_chip->IO_ADDR_R   = "NFDATA的虚拟地址";  //读函数中用到的地址
	s3c_nand_chip->IO_ADDR_W   = "NFDATA的虚拟地址";  //写函数中用到的地址
  	s3c_nand_chip->dev_ready   = s3c2440_dev_ready;
	/* 3.硬件相关的操作 */



	/* 4.使用:nand_scan */
	/* 4.1 nand_scan函数要用到mtd_info结构，所以分配一个 */
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	/* 4.2将nand_chip结构和mtd_info结构联系起来 */
	s3c_mtd->priv = s3c_nand_chip;  
	s3c_mtd->owner = THIS_MODULE;
	/* nand_scan根据nand_chip的底层操作函数识别NAND FLASH，设置mtd_info结构（读写擦除） */
	nand_scan(s3c_mtd, 1);  //参2表示芯片个数

	
	/* 5.增加mtd分区：add_mtd_partitons */
	

	
	return 0;
}

/* 出口函数 */
static void s3c_nand_exit(void)
{

}


module_init(s3c_nand_init);
module_exit(s3c_nand_exit);
MODULE_LICENSE("GPL");












