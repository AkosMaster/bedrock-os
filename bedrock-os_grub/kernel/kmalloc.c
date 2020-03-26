#include "kmalloc.h"
#include "../drivers/screen.h"

uint8_t k_memory_map[k_memory_map_entries];

uint32_t kmalloc(uint32_t block_count) {

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

	    return k_free_mem_addr + starting_block_id * block_size;
	}

    }   

    return 0;
}

void kfree(uint32_t address, uint32_t block_count) {
    uint32_t c;
    uint32_t starting_block = (address - k_free_mem_addr) / block_size;
    for (c = starting_block; c < starting_block + block_count; c++) {
	k_memory_map[c] = 0;
    }
}

void kmalloc_test() {
    kprint("allocating 1 page\n");

    uint32_t address = kmalloc(1);
    if (address != 0) {
	kprint("kmalloc address: ");
	kprint_hex(address);
	kprint("\n");
	kprint("freeing page\n");
	kfree(address, 1);
    } else {
	kprint("no free pages left\n");
    }
    kprint("[test finished]\n");
}
