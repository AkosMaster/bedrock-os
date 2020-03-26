#include "vm.h"
#include "kmalloc.h"
#include "../drivers/screen.h"
#include "../cpu/gdt.h"

uint32_t kernel_page_directory[1024] __attribute__((aligned(4096)));

/* substract 0xc0000000 from address, since kernel (phys 0x0) is mapped to 0xc0000000 (virtual)*/
uint8_t* kernel_to_phys_addr(uint8_t* address) {
    address -= 0xc0000000;
    return address;
}

uint8_t* phys_to_kernel_addr(uint8_t* address) {
    address += 0xc0000000;
    return address;
}

void clear_entries (uint32_t *table) {

    //set each entry to not present
    int i;
    for(i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
	//   Supervisor: Only kernel-mode can access them
	//   Write Enabled: It can be both read from and written to
	//   Not Present: The page table is not present
	table[i] = 0x00000002;
    }

}

void identity_map_page_table (uint32_t *page_table, unsigned int flags) {

    // holds the physical address where we want to start mapping these pages to.
    // in this case, we want to map these pages to the very beginning of memory.
    unsigned int i;
 
    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for(i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        page_table[i] = (i * 0x1000) | flags; // attributes: supervisor level, read/write, present.
    }

}

uint32_t strip_vm_flags(uint32_t address) {
    return address - address % 0x1000;
}

int map_page(uint32_t *page_directory, void * physaddr, void * virtualaddr, unsigned int flags)
{
    // Make sure that both addresses are page-aligned.
    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;
 
    uint32_t page_table_ptr = (uint32_t)phys_to_kernel_addr((uint8_t *)page_directory[pdindex]);
    page_table_ptr = strip_vm_flags(page_table_ptr);

    // is entry present?
    if (page_directory[pdindex] == 0x00000002) {
	// if not, let's create one

	page_table_ptr = (unsigned long)kmalloc(1);

	clear_entries((uint32_t *)page_table_ptr);
	page_directory[pdindex] = ((unsigned long) kernel_to_phys_addr((uint8_t *)page_table_ptr)) | (flags & 0xFFF) | 0x01;
    }

    unsigned long * page_table = (unsigned long *) page_table_ptr;

    // is entry present?
    if (page_table[ptindex] != 0x00000002) {
	return 1; // already present, don't modify
    }

    page_table[ptindex] = ((unsigned long)physaddr) | (flags & 0xFFF) | 0x01; // Present
 
    //flush_tlb();
    return 0;
}

void flush_tlb() {
    asm(
	"movl	%cr3,%eax\n\t"
	"movl	%eax,%cr3\n\t"
    );
}

void load_page_directory(uint8_t *page_directory) {

    asm (
	"mov 8(%esp), %eax \n\t"
	"mov %eax, %cr3 \n\t"
    );

}

void setup_paging () {

    kprint("setting up paging");

    uint32_t first_page_table[1024] __attribute__((aligned(4096))); 
    clear_entries (first_page_table);

    clear_entries (kernel_page_directory);
    identity_map_page_table(first_page_table, 3);

    // attributes: supervisor level, read/write, present
    kernel_page_directory[0] = ((unsigned int)kernel_to_phys_addr((uint8_t *)&first_page_table) ) | 3;
    kernel_page_directory[0x300] = ((unsigned int)kernel_to_phys_addr((uint8_t *)&first_page_table) ) | 3;

    uint8_t *page_dir_ptr = (uint8_t *)&kernel_page_directory;
    load_page_directory(kernel_to_phys_addr((uint8_t *)page_dir_ptr));

    /* Logging */

    	kprint("\n\\ (PD addr.: ");
    	kprint_hex((uint32_t)page_dir_ptr);
    	kprint(")\n");

    kprint("\\ removing unused page table");
    // the 0th entry is not needed anymore so we can remove it.
    // kernel is mapped at 0xc0000000 only from now on.
    kernel_page_directory[0] = 0x00000002;

    kprint_at_col(STATUS_OK_MSG, STATUS_COL);
}

