@******************************************************************************
@ File：starrt.S
@ 功能：通过它转入C程序
@******************************************************************************       

.text
.global _start
_start:
			/* 1.关闭看门狗 */
            ldr     r0, =0x53000000       @ WATCHDOG寄存器地址
            mov     r1, #0x0                     
            str     r1, [r0]              @ 写入0，禁止WATCHDOG，否则CPU会不断重启
           
		    /* 2.设置时钟 */
			/* 2.1 设置LOCKTIME(0x4C000000) = 0xFFFFFFFF */
			ldr     r0, =0x4C000000      
            mov     r1, #0xFFFFFFFF                     
            str     r1, [r0]             
            /* 2.2 设置CLKDIVN(0x4C000014) = 0X5, tFCLK:tHCLK:tPCLK = 1:4:8*/
			ldr     r0, =0x4C000014    
            mov     r1, #0x5                    
            str     r1, [r0]  
			/* 2.3 设置cpu工作与异步模式*/
			mrc     p15,0,r0,c1,c0,0
			orr     r0,r0,#0xc0000000 //R1_nF:OR:R1_iA
			mcr     p15,0,r0,c1,c0,0
			/* 2.4 设置MPLLCON(0x4C000004) = (92<<12)|(1<<4)|(1<<0)*/
			ldr     r0, =0x4C000004   
            ldr     r1, =(92<<12)|(1<<4)|(1<<0)                   
            str     r1, [r0] 
			
			/* 3.设置栈 */
			mov 	r1, #0
			ldr 	r0, [r1]              /* 读出原来的值备份 */
			str 	r1, [r1]              /* 0->[0] */
			ldr 	r2, [r1]              /* r2=[0] */
			cmp 	r1, r2                /* r1==r2? 如果相等表示是NAND启动 */
			ldr 	sp, =0x40000000+4096  /* 先假设是nor启动 */
			moveq 	sp, #4096             /* nand启动 */
			streq 	r0, [r1]              /* 恢复原来的值 */
			                                       
            bl      main                  @ 调用C程序中的main函数
halt_loop:
            b       halt_loop
