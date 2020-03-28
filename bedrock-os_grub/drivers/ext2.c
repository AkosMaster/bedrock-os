#include "ext2.h"
#include "storage.h"
#include "screen.h"
#include "../kernel/kmalloc.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#include "../kernel/kernel.h"

#define EXT2_SUPERBLOCK 2
#define EXT2_SIGNATURE 0xef53
#define ROOT_INODE 2

char* inode_type_names[] = {
    "FIFO",
    "Character device",
    "Directory",
    "Block device",
    "Regular file",
    "Symbolic link",
    "Unix socket"
};

char* get_inode_type_name (uint16_t type) {
    return inode_type_names[(type/0x1000) / 2];
}

void ext2_read_block(struct device_info *device, struct ext2_priv_data *priv, uint32_t block, uint8_t *buf)
{
    uint32_t sectors_per_block = priv->sectors_per_block;
    /*mprint("we want to read block %d which is sectors [%d; %d]\n",
        block, block*sectors_per_block , block*sectors_per_block + sectors_per_block);*/
    //kprintf("  %d", block);
    read_sectors(device, block*sectors_per_block, sectors_per_block, buf);

}

void ext2_read_inode(struct device_info *dev, struct ext2_priv_data *priv, uint32_t inode, struct inode *inode_buf)
{
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	/* Now we have which BG the inode is in, load that desc */
	uint8_t * block_buf = (uint8_t *)kmalloc(1);
	ext2_read_block(dev, priv, priv->first_bgd, block_buf);
	struct block_group *bgd = (struct block_group *)block_buf;
	kprint("We seek BG "); kprint_int(bg); kprint("\n");
	/* Seek to the BG's desc */
	for(i = 0; i < bg; i++)
		bgd++;
	/* Find the index and seek to the inode */
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	kprint("Index of our inode is "); kprint_int(index); kprint("\n");
	uint32_t block = (index * sizeof(struct inode))/ priv->blocksize;
	kprint("Absolute: "); kprint_int(bgd->block_of_inode_table + block); kprint("\n");
	ext2_read_block(dev, priv, bgd->block_of_inode_table + block, block_buf);
	struct inode* _inode = (struct inode *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	/* We have found the inode! */
	memory_copy((uint8_t*)_inode, (uint8_t*)inode_buf, sizeof(struct inode));
}

uint32_t ext2_read_directory(struct device_info *dev, struct ext2_priv_data *priv, char *filename, struct ext2_dir_entry *dir)
{
	while(dir->inode != 0) {
		char *name = (char *)kmalloc(1);
		memory_copy(&dir->reserved+1, name, dir->namelength);
		name[dir->namelength] = 0;
		//kprintf("DIR: %s (%d)\n", name, dir->size);
		if(filename && strcmp(filename, name) == 0)
		{
			/* If we are looking for a file, we had found it */
			//ext2_read_inode(dev, priv, inode, dir->inode);
			kprint("Found inode "); kprint(filename); kprint(": "); kprint_int(dir->inode); kprint("\n");
			kfree((uint32_t *)name, 1);
			return dir->inode;
		}
		if(!filename && (uint32_t)filename != 1) {
			//mprint("Found dir entry: %s to inode %d \n", name, dir->inode);
			kprint(name); kprint("\n");
		}
		dir = (struct ext2_dir_entry *)((uint32_t)dir + dir->size);
		kfree((uint32_t *)name, 1);
	}
	return 0;
}

uint8_t ext2_read_root_directory(struct device_info *dev, struct ext2_priv_data *priv, char *filename, uint8_t* root_buf)
{
	/* The root directory is always inode#2, so find BG and read the block. */
	struct inode *inode = (struct inode *)kmalloc(1);
	ext2_read_inode(dev, priv, 2, inode);
	if((inode->type & 0xF000) != 0x4000)
	{
		kprint("FATAL: Root directory is not a directory!\n");
                kfree((uint32_t *)inode, 1);
		return 0;
	}
	/* We have found the directory!
	 * Now, load the starting block
	 */
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(b == 0) break;
		ext2_read_block(dev, priv, b, root_buf);
		/* Now loop through the entries of the directory */
		if(ext2_read_directory(dev, priv, filename, (struct ext2_dir_entry*)root_buf)) { 
		    kfree((uint32_t *)inode, 1);
		    return 1; 
		}
	}

        kfree((uint32_t *)inode, 1);

	if(filename && (uint32_t)filename != 1) return 0;
	return 1;
}

uint8_t ext2_find_file_inode(struct device_info *dev, struct ext2_priv_data *priv, char *ff, struct inode *inode_buf)
{
        struct inode *inode = (struct inode *)kmalloc(1);
        uint8_t *root_buf = (uint8_t*)kmalloc(1);
	char *filename = (char *)kmalloc(1);
	memory_copy((uint8_t*)ff, (uint8_t*)filename, strlen(ff) +1);

	int n = strsplit(filename, '/');
	filename ++; // skip the first crap
	uint32_t retnode = 0;
        
	if(n > 1)
	{ 
		/* Read inode#2 (Root dir) into inode */
		ext2_read_inode(dev, priv, 2, inode);
		/* Now, loop through the DPB's and see if it contains this filename */
		n--;
		while(n--)
		{
			kprint("Looking for: "); kprint(filename); kprint("\n");
			for(int i = 0; i < 12; i++)
			{
				uint32_t b = inode->dbp[i];
				if(!b) {break;};
				ext2_read_block(dev, priv, b, root_buf);
				uint32_t rc = ext2_read_directory(dev, priv, filename, (struct ext2_dir_entry *)root_buf);
                                			
				if(!rc)
				{

					if(strcmp(filename, "") == 0)
					{
						kfree((uint32_t *)filename, 1);
						kfree((uint32_t *)inode, 1);
						kfree((uint32_t *)root_buf, 1);
						return strcmp(ff, "/")?retnode:1;
					}
					kprint("File not found: "); kprint(filename); kprint("\n");
					kfree((uint32_t *)filename, 1);
					kfree((uint32_t *)inode, 1);
					kfree((uint32_t *)root_buf, 1);
					return 0;
				} else {
					/* inode now contains that inode
					 * get out of the for loop and continue traversing
					 */
					 ext2_read_inode(dev, priv, rc, inode); // read found inode
					 retnode = rc;
					 break;
					 //goto fix;
				}
				
			}
			fix:;
			uint32_t s = strlen(filename);
			filename += s + 1;
		}
		memory_copy((uint8_t*)inode, (uint8_t*)inode_buf, sizeof(struct inode));
	} else {
		/* This means the file is in the root directory */
		ext2_read_root_directory(dev, priv, filename, (uint8_t*)inode);
                
		memory_copy((uint8_t*)inode, (uint8_t*)inode_buf, sizeof(struct inode));
	}
	kfree((uint32_t *)filename, 1);
	kfree((uint32_t *)inode, 1);
	kfree((uint32_t *)root_buf, 1);
	return retnode;
}

void fill_priv_data(struct superblock *sb ,struct ext2_priv_data *priv) {
    int blocksize = 1024 << sb->blocksize_hint;

    priv->blocksize = blocksize;
    priv->inodes_per_block = blocksize / sizeof(struct inode);
    priv->sectors_per_block = blocksize / 512;

    priv->first_bgd = sb->superblock_id + (sizeof(struct superblock) / blocksize);
    priv->number_of_bgs = sb->blocks / sb->blocks_in_blockgroup;

    memory_copy((uint8_t*)sb, (uint8_t*)&priv->sb, sizeof(struct superblock));
}

void ext2_init(struct device_info *device) {

    kprint("\\ initializing ext2\n");
    kprint("\\ loading superblock\n");

    struct superblock *sb = (struct superblock*)kmalloc(1);

    // read superblock starting at sector 2 taking up two sectors
    read_sectors(device, EXT2_SUPERBLOCK, 2, sb);

    struct ext2_priv_data *priv = (struct ext2_priv_data *)kmalloc(1);
    fill_priv_data(sb, priv);

    if (sb->ext2_sig != 0xef53) {
	kprint("\\ error: invalid ext2 signature\n");
	return;
    } else {kprint("\\ valid ext2 signature\n");}

    /* print info */

    kprint("\\ major version: ");
    kprint_int(sb->major_version);
    kprint("\n");

    #define SYSCFG "sys.cfg"

    struct inode *syscfg_inode = (struct inode *)kmalloc(1);
    ext2_find_file_inode(device, priv, SYSCFG, syscfg_inode);

    if ((syscfg_inode->type & 0xF000) != 0x8000) {
        kprint("\\ missing file: "); kprint(SYSCFG); kprint("\n");
	kernel_exit(3);
    }

    /* free allocated memory */
    kfree((uint32_t *)syscfg_inode, 1);
    kfree((uint32_t *)sb, 1);
    kfree((uint32_t *)priv, 1);

}
