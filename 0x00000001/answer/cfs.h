#ifndef INCLUDED_CFS_H
#define INCLUDED_CFS_H

#include <sys/types.h>
#include <assert.h>

#define ALIGNUP_4096(x) (((x) + 0xfff) & 0xfffff000)

/* Yes! `(__uint32_t) 4096` also works, but this is better than it */
#define BLOCK_SIZE 4096UL


#define try(expr) assert(expr >= 0)

#define checkp(p) assert(p != NULL)

#define inode_bmap(inode, bn) ((inode)->blocks[bn])

#define IMD_AP_ROOT_RD 0x0001
#define IMD_AP_ROOT_WR 0x0002
#define IMD_AP_ROOT_XR 0x0004

#define IMD_AP_USER_RD 0x0008
#define IMD_AP_USER_WR 0x0010
#define IMD_AP_USER_XR 0x0030

#define IMD_FT_REGFILE 0x0040
#define IMD_FT_FOLDER  0x0080
#define IMD_FT_BLKDEV  0x0100
#define IMD_FT_CHRDEV  0x0200
#define IMD_FT_POINTER 0x0400

/* NOTE! LBA address should be 24 or 48 bits, but we use 32bit (because we use block) */


struct inode {
	/* inode */
	__uint32_t inode;

	/* inode of its parent */
	__uint32_t pinode;

	/* inode mode */
	__uint16_t mode;

	/* block count (if it's a pointer, point to inode)*/
	__uint32_t block_count;

	/* offset to last block (block_count * 4096 + block_offset = file_size) */
	__uint16_t boffset;

	/* block pointers */
	__uint32_t blocks[8];

	/* now, there was 48 bytes, I want to use 64 bytes */

	/* reserved */
	char reserved[16];
} __attribute__ ((packed));


struct dir {
	char names[0];
};

struct super_block {
	/* the first sector should be reserved anytime. */
	unsigned char mbr[512];

	/* 0x1328e3b */

#define SB_MAGIC 0x1328e3b

	__uint32_t magic;

	/* block count */
	__uint32_t cblocks;

	/* block bitmap */
	__uint32_t block_bitmap;

	/* inodes start block */
	__uint32_t inodes;

	__uint32_t cinodes;

	__uint32_t first_datablock;
#define SB_RESERVED_SIZE BLOCK_SIZE - 512 - 6 * 4

	char reserved[SB_RESERVED_SIZE];
} __attribute__((packed)) superblock_d;


struct cfs {
	/* a superblock */
	struct super_block *superblock;

	/* block bitmap */
	__uint8_t *block_bitmap;

	/* inode bitmap */
	__uint8_t *inode_bitmap;

	/* and inodes */
	struct inode *inodes;
} cfs;


#endif
