#include "kernel.h"

#include "kmalloc.h"
#include "../cpu/gdt.h"
#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../drivers/storage.h"
#include "../drivers/ext2.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "vm.h"
#include "kmalloc.h"
#include "ring3.h"
#include <stdint.h>

void setup_interrupt_handlers() {
    kprint("installing interrupt handlers");
    isr_install();
    irq_install();
    kprint_at_col(STATUS_OK_MSG, STATUS_COL);
}

void init_filesystem(struct device_info device) {

    kprint("initializing filesystem on device ");
    kprint_int(device.vdID);
    kprint("\n");

    ext2_init(device);

    kprint("\\ filesystem loaded");
    kprint_at_col(STATUS_OK_MSG, STATUS_COL);
}

char empty_sector[512];

void kernel_main(uint8_t bootmode) {

    clear_screen();
    kprint("bedrock-os v0.3 (alpha build)\n");

    /* kernel stack is in first kmalloc block and interrupt stack is in the second */
    k_memory_map[0] = 1; // lock both blocks
    k_memory_map[1] = 1;

    setup_gdt();
    set_kernel_stack(k_free_mem_addr + block_size*2 - 16); // ~0x1000 large stack for interrupts
    setup_paging();
    setup_interrupt_handlers();

    storage_driver_init();
    init_filesystem(vdevices[0]);

    kprint("[setup finished]\n");

    kernel_exit( 0 );
}

void user_input(char *input) {

    if (strcmp(input, "hlt") == 0) {
	asm("hlt");
    } else if (strcmp(input, "kmalloc") == 0) {
	kmalloc_test();
    } else if (strcmp(input, "int") == 0) {
        asm("int $1");
        kprint("[test finished]\n");
    } else {
	kprint("unknown command\n");
    }

    asm("int $42");

    kprint("sys-dbg> ");
}

char *exit_messages[] = {
    "finished",
    "panic",
    "unknown error"
};

void kernel_exit(uint8_t ERROR_CODE) {

    kprint("\nkernel exited with error code: ");
    kprint_hex(ERROR_CODE);
    kprint(" (");
    kprint(exit_messages[ERROR_CODE]);
    kprint(")\n");

    kprint("sys-dbg> ");

    for (;;) {}

}
