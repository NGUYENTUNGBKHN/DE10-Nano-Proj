SEARCH_DIR(.)

MEMORY
{
  boot_rom (rx) : ORIGIN = 0xfffd0000, LENGTH = 64K
  oc_ram (rwx) : ORIGIN = 0xffff0000, LENGTH = 64K
  ram (rwx) : ORIGIN = 0x100000, LENGTH = 1023M
}

SECTIONS
{
	.text :
  	{
  		*(.text .text.* .gnu.linkonce.t.*)
	} >ram
	.rodata : ALIGN (4)
  	{
    	*(.rodata .rodata.* .gnu.linkonce.r.*)
  	} >ram
  	
  	.data : 
  	{
  		*(.data)
  	} >ram
  	
  	.bss :
  	{
  		*(.bss)
  	} >ram
  	/* --- ADD THE DEFINITION HERE --- */
    _end = .;
    
    /* Define the end of the memory region, often used for stack initialisation */
    _stack = ORIGIN(ram) + LENGTH(ram);
}

