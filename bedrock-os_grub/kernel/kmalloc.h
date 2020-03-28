#include <stdint.h>
#include <stddef.h>


#define BLOCK_SIZE 0x1000
#define k_memory_map_entries 100

uint32_t* kmalloc(uint32_t block_count);
void kfree(uint32_t* address, uint32_t block_count);
uint8_t k_memory_map[k_memory_map_entries];
void kmalloc_test();
