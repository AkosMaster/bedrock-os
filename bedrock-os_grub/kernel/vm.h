#include <stdint.h>
#include <stddef.h>

uint32_t kernel_page_directory[1024] __attribute__((aligned(4096)));

void clear_entries (uint32_t *table);
void identity_map_page_table (uint32_t *page_table, unsigned int flags);
void load_page_directory(uint8_t *page_directory);

int map_page(uint32_t *page_directory, void * physaddr, void * virtualaddr, unsigned int flags);

uint8_t* kernel_to_phys_addr(uint8_t* address);
uint8_t* phys_to_kernel_addr(uint8_t* address);

void setup_paging();
