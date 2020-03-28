#include "kmalloc.h"
#include "kernel.h"
#include "../libc/mem.h"
#include "../drivers/screen.h"

uint8_t k_memory_map[k_memory_map_entries];
char *kmalloc_memory[BLOCK_SIZE*(k_memory_map_entries + 10)] __attribute__ ((aligned (0x1000)));

uint8_t *empty_block[BLOCK_SIZE];

uint32_t* kmalloc(uint32_t block_count) {

    uint32_t found_block_count = 0;
    uint32_t current_block_id;
    uint32_t starting_block_id = 0;

    for (current_block_id = 0; current_block_id < k_memory_map_entries; current_block_id++) {

	if (k_memory_map[current_block_id] == 0) { // if block is free

	    if (found_block_count == 0) { // if this is the first block in the sequence
		starting_block_id = current_block_id;
	    }

	    found_block_count++;
	}

	if (found_block_count == block_count) { // found a long enough sequence

	    int c;
	    for (c = starting_block_id; c < starting_block_id + block_count; c++) {
		k_memory_map[c] = 1; // lock all blocks in sequence
	    }

            uint32_t block_address = ((uint32_t)(&kmalloc_memory) + starting_block_id * BLOCK_SIZE);

	    memory_set((uint8_t*) block_address, 0, BLOCK_SIZE); // clear block

            //kprint("kmalloc "); kprint_hex(block_address); kprint("\n");

	    return (uint32_t*) block_address;
	}

    }   
    
    kernel_exit(2);
    return 0;
}

void kfree(uint32_t *address, uint32_t block_count) {
    //kprint("kfree "); kprint_hex(address); kprint("\n");
    uint32_t c;
    uint32_t starting_block = ((uint32_t)address - (uint32_t)&kmalloc_memory) / BLOCK_SIZE;
    for (c = starting_block; c < starting_block + block_count; c++) {
	k_memory_map[c] = 0;
    }
}

void kmalloc_test() {
    kprint("allocating 1 page\n");

    kprint("page 0 address: "); kprint_hex((uint32_t)&kmalloc_memory); kprint("\n");

    uint32_t* address = kmalloc(1);
    if (address != 0) {
	kprint("kmalloc address: ");
	kprint_hex((uint32_t)address);
	kprint("\n");
	kprint("freeing page\n");
	kfree(address, 1);
    } else {
	kprint("no free pages left\n");
    }
    kprint("[test finished]\n");
}
