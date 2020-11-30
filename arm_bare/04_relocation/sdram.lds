
/*
格式如下，其中secname和contents是必须的，secname是段名，start运行地址，AT存储地址，
contents可以是完整目标或者其中的某段。

SECTIONS{
	...
	secname start BLOCK(align) (NOLOAD) : AT(ldadr)
	{contents} >region : phdr = fill
	...
}
*/


SECTIONS{
	. = 0x30000000;
	
	. =  ALIGN(4);
	.text  : {*(.text)}
	
	. =  ALIGN(4);
	.rodata : {*(.rodata)}
	
	. =  ALIGN(4);
	.data : {*(.date)}
	
	. =  ALIGN(4);
	__bss_start = .;
	.bss : {*(.bss	) *(.COMMON)}
	_end = .;
	
	}