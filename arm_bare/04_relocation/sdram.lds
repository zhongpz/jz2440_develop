
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
	.text 0 : {*(.text)}
	.rodata : {*(.rodata)}
	.data 0x30000000 : AT(0X700) 
	{
		data_load_addr = LOADADDR(.data);
		data_start = .;
		*(.data)
		data_end = .;
	}
	bss_start = .;
	.bss : {*(.bss	) *(.COMMON)}
	bss_end = .;
	
	}