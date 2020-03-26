#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>
#include <stddef.h>

#define k_free_mem_addr 0xC0010000
#define block_size 0x1000
#define k_memory_map_entries 10

uint32_t kmalloc(uint32_t block_count);
uint8_t k_memory_map[k_memory_map_entries];
void kmalloc_test();

#endif
