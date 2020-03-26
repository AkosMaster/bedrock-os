#include "ext2.h"
#include "storage.h"
#include "screen.h"

char block_buff[512*4];

#define EXT2_SUPERBLOCK 2
#define EXT2_SECTORS_PER_BLOCK 1
#define EXT2_SIGNATURE 0xef53

struct fs_info {
    uint32_t sectors_per_block;
    uint32_t inodes_per_group;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t inode_count;
    uint16_t signature;
};

struct fs_info get_fs_info (uint8_t* supernode) {

    struct fs_info info;

    info.inodes_per_group = *((uint32_t*)&supernode[40]);
    info.block_size = 1024 << *((uint32_t*)&supernode[24]);
    info.block_count = *((uint32_t*)&block_buff[4]);
    info.inode_count = *((uint32_t*)&block_buff[0]);
    info.signature = *((uint16_t*)&block_buff[56]);

    return info;
}

void ext2_init(struct device_info device) {

    kprint("\\ initializing ext2\n");

    read_sector(device, EXT2_SUPERBLOCK, block_buff);

    struct fs_info fs = get_fs_info(block_buff);

    if (fs.signature != 0xef53) {
	kprint("\\ error: invalid ext2 signature\n");
	return;
    } else {kprint("\\ valid ext2 signature\n");}

    /* print info */

    kprint("\\ number of inodes: ");
    kprint_int(fs.inode_count);
    kprint("\n");

    kprint("\\ number of blocks: ");
    kprint_int(fs.block_count);
    kprint("\n");

    kprint("\\ block size: ");
    kprint_int(fs.block_size);
    kprint(" bytes\n");

    

}
