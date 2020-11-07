#ifndef S3C2440_H_
#define S3C2440_H_

/*1.GPIO类*/
#define	GPFCON		(*(volatile unsigned long *)0x56000050)
#define	GPFDAT		(*(volatile unsigned long *)0x56000054)

#define GPHCON (*(volatile unsigned long *)0x56000070)
#define GPHUP (*(volatile unsigned long *)0x56000078)


/*2.串口类*/
#define ULCON0 (*(volatile unsigned long *)0x50000000)
#define UCON0 (*(volatile unsigned long *)0x50000004)

#define UBRDIV0 (*(volatile unsigned long *)0x50000028)

#define UTRSTAT0 (*(volatile unsigned long *)0x50000010)
#define UTXH0 (*(volatile unsigned long *)0x50000020)
#define URXH0 (*(volatile unsigned long *)0x50000024)

/*3.内存控制器类*/
#define BANKCON0 (*(volatile unsigned long *)0x48000004)
#define BANKCON6 (*(volatile unsigned long *)0x4800001C)
#define BWSCON (*(volatile unsigned long *)0x48000000)
#define REFRESH (*(volatile unsigned long *)0x48000024)
#define BANKSIZE (*(volatile unsigned long *)0x48000028)
#define MRSRB6 (*(volatile unsigned long *)0x4800002C)



#endif
