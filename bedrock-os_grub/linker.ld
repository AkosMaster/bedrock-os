ENTRY(_start)

SECTIONS {

    .kernel_entry 0x0000 : AT(0x0000)
    {
	boot/*.o(.text)
    }

    .text 0xC0001000 : AT(0x00001000)
    {
	kernel/*.o(.text)
    }

    .rodata 0xC0050000 : AT(0x00050000)
    {
       kernel/*.o(.rodata)
    }

}
