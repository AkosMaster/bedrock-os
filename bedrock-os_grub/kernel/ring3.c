#include "ring3.h"
#include "kernel.h"

void userland_entry() {
    
    for (;;) {}
}

void jump_usermode(void* entrypoint, uint32_t new_esp) {
    asm(
	"_jump_usermode:\n\t"
	"mov 8(%esp), %ebx\n\t"
	"mov 12(%esp), %ecx\n\t"

     	"mov $0x23, %ax\n\t"
     	"mov %ax, %ds\n\t"
     	"mov %ax, %es\n\t"
     	"mov %ax, %fs\n\t"
     	"mov %ax, %gs\n\t"

     	"mov %ecx, %eax\n\t"
     	"push $0x23\n\t"
     	"push %eax\n\t"
     	"pushf\n\t"
     	"push $0x1B\n\t"
     	"push %ebx\n\t"
     	"iret\n\t"
    );
}

void enter_ring3 () {

    jump_usermode(userland_entry, 0x30000);

}
