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

void init_filesystem(struct device_info *device) {

    kprint("initializing filesystem on device ");
    kprint_int(device->vdID);
    kprint("\n");

    ext2_init(device);

    kprint("\\ filesystem loaded");
    kprint_at_col(STATUS_OK_MSG, STATUS_COL);
}

void kernel_main(uint8_t bootmode) {

    clear_screen();
    kprint("bedrock-os v0.3 (alpha build)\n");

    /* kernel stack */
    kmalloc(2);

    setup_gdt();
    set_kernel_stack((uint32_t)kmalloc(2) + BLOCK_SIZE*2 - 16); // ~0x2000 large stack for interrupts
    setup_paging();
    setup_interrupt_handlers();

    //for (int x = 0; x < 200; x++) {kmalloc(1);}

    storage_driver_init();
    init_filesystem(&vdevices[0]);

    kprint("[setup finished]\n");

    kmalloc_test();

    kernel_exit( 0 );
}

void user_input(char *input) {

}

char *exit_messages[] = {
    "finished",
    "unknown error",
    "out of kernel memory",
    "missing system file"
};

void kernel_exit(uint8_t ERROR_CODE) {

    kprint("\nkernel exited with error code: ");
    kprint_hex(ERROR_CODE);
    kprint(" (");
    kprint(exit_messages[ERROR_CODE]);
    kprint(")\n");

    for (;;) {}

}
