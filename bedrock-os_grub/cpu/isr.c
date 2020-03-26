#include "isr.h"
#include "idt.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "timer.h"
#include "ports.h"
#include "../drivers/ATA.h"
#include "../drivers/storage.h"

isr_t interrupt_handlers[256];

/* Can't do this with a loop because we need the address
 * of the function names */
void isr_install() {

    #define FLAGS_RING0_ONLY 0x8E
    #define FLAGS_USERMODE 0xEE

    set_idt_gate(0, (uint32_t)isr0, FLAGS_RING0_ONLY);
    set_idt_gate(1, (uint32_t)isr1, FLAGS_USERMODE);
    set_idt_gate(2, (uint32_t)isr2, FLAGS_RING0_ONLY);
    set_idt_gate(3, (uint32_t)isr3, FLAGS_RING0_ONLY);
    set_idt_gate(4, (uint32_t)isr4, FLAGS_RING0_ONLY);
    set_idt_gate(5, (uint32_t)isr5, FLAGS_RING0_ONLY);
    set_idt_gate(6, (uint32_t)isr6, FLAGS_RING0_ONLY);
    set_idt_gate(7, (uint32_t)isr7, FLAGS_RING0_ONLY);
    set_idt_gate(8, (uint32_t)isr8, FLAGS_RING0_ONLY);
    set_idt_gate(9, (uint32_t)isr9, FLAGS_RING0_ONLY);
    set_idt_gate(10, (uint32_t)isr10, FLAGS_RING0_ONLY);
    set_idt_gate(11, (uint32_t)isr11, FLAGS_RING0_ONLY);
    set_idt_gate(12, (uint32_t)isr12, FLAGS_RING0_ONLY);
    set_idt_gate(13, (uint32_t)isr13, FLAGS_RING0_ONLY);
    set_idt_gate(14, (uint32_t)isr14, FLAGS_RING0_ONLY);
    set_idt_gate(15, (uint32_t)isr15, FLAGS_RING0_ONLY);
    set_idt_gate(16, (uint32_t)isr16, FLAGS_RING0_ONLY);
    set_idt_gate(17, (uint32_t)isr17, FLAGS_RING0_ONLY);
    set_idt_gate(18, (uint32_t)isr18, FLAGS_RING0_ONLY);
    set_idt_gate(19, (uint32_t)isr19, FLAGS_RING0_ONLY);
    set_idt_gate(20, (uint32_t)isr20, FLAGS_RING0_ONLY);
    set_idt_gate(21, (uint32_t)isr21, FLAGS_RING0_ONLY);
    set_idt_gate(22, (uint32_t)isr22, FLAGS_RING0_ONLY);
    set_idt_gate(23, (uint32_t)isr23, FLAGS_RING0_ONLY);
    set_idt_gate(24, (uint32_t)isr24, FLAGS_RING0_ONLY);
    set_idt_gate(25, (uint32_t)isr25, FLAGS_RING0_ONLY);
    set_idt_gate(26, (uint32_t)isr26, FLAGS_RING0_ONLY);
    set_idt_gate(27, (uint32_t)isr27, FLAGS_RING0_ONLY);
    set_idt_gate(28, (uint32_t)isr28, FLAGS_RING0_ONLY);
    set_idt_gate(29, (uint32_t)isr29, FLAGS_RING0_ONLY);
    set_idt_gate(30, (uint32_t)isr30, FLAGS_RING0_ONLY);
    set_idt_gate(31, (uint32_t)isr31, FLAGS_RING0_ONLY);

    // Remap the PIC
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0); 

    // Install the IRQs
    set_idt_gate(32, (uint32_t)irq0, 0x8E);
    set_idt_gate(33, (uint32_t)irq1, 0x8E);
    set_idt_gate(34, (uint32_t)irq2, 0x8E);
    set_idt_gate(35, (uint32_t)irq3, 0x8E);
    set_idt_gate(36, (uint32_t)irq4, 0x8E);
    set_idt_gate(37, (uint32_t)irq5, 0x8E);
    set_idt_gate(38, (uint32_t)irq6, 0x8E);
    set_idt_gate(39, (uint32_t)irq7, 0x8E);
    set_idt_gate(40, (uint32_t)irq8, 0x8E);
    set_idt_gate(41, (uint32_t)irq9, 0x8E);
    set_idt_gate(42, (uint32_t)irq10, 0x8E);
    set_idt_gate(43, (uint32_t)irq11, 0x8E);
    set_idt_gate(44, (uint32_t)irq12, 0x8E);
    set_idt_gate(45, (uint32_t)irq13, 0x8E);
    set_idt_gate(46, (uint32_t)irq14, 0x8E);
    set_idt_gate(47, (uint32_t)irq15, 0x8E);

    set_idt(); // Load with ASM
}

/* To print the message which defines every exception */
char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(registers_t *r) {
    kprint("received interrupt: ");
    kprint_int(r->int_no);
    kprint(" (");
    kprint(exception_messages[r->int_no]);
    kprint(")\n");

    kprint("error eip: ");
    kprint_hex(r->eip);
    kprint("\ninstruction: ");

    uint32_t * instr = (uint32_t *) r->eip;

    kprint_hex(instr[0]);
    kprint("\n");

    kprint("error code: \n");
    int bit_0 = (r->err_code & ( 1 << 0 )) >> 0;
    int bit_1 = (r->err_code & ( 1 << 1 )) >> 1;
    int bit_2 = (r->err_code & ( 1 << 2 )) >> 2;
    int bit_3 = (r->err_code & ( 1 << 3 )) >> 3;
    int bit_4 = (r->err_code & ( 1 << 4 )) >> 4;  

    kprint("bit 0: ");
    kprint_hex(bit_0);
    kprint("\n");
    kprint("bit 1: ");
    kprint_hex(bit_1);
    kprint("\n");
    kprint("bit 2: ");
    kprint_hex(bit_2);
    kprint("\n");
    kprint("bit 3: ");
    kprint_hex(bit_3);
    kprint("\n");
    kprint("bit 4: ");
    kprint_hex(bit_4);
    kprint("\n");

    kprint("value of cr2: ");
    uint32_t cr2;
    asm(
	"mov %%cr2, %%eax;"
        "mov %%eax, %0;"
	:"=r"(cr2)
    :);
    kprint_hex(cr2);

    kprint("\n");

    /* print stack pointer */
    kprint("stack pointer: ");
    void* p = 0;
    kprint_hex((uint32_t)&p);
    kprint("\n");

    asm("hlt");
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t *r) {
    ide_irq(); /* ATA driver needs this */

    /* After every interrupt we need to send an EOI to the PICs
     * or they will not send another interrupt again */
    if (r->int_no >= 40) port_byte_out(0xA0, 0x20); /* slave */
    port_byte_out(0x20, 0x20); /* master */

    /* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }
}

void irq_install() {
    /* Enable interruptions */
    asm volatile("sti");
    /* IRQ0: timer */
    init_timer(TIMER_FREQ);
    /* IRQ1: keyboard */
    init_keyboard();
}
