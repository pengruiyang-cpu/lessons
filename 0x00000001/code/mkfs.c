#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bitmap.h"
#include "cfs.h"


#define panic(s) { \
	printf("mkfs: "s); \
	printf("\nusage: \n\tmkfs <device>\n"); \
	exit(1); \
}

#define bpwrite(fd, buffer, size, bn) (pwrite(fd, buffer, size, bn * BLOCK_SIZE))
#define bpread(fd, buffer, size, bn) (pread(fd, buffer, size, bn * BLOCK_SIZE))


int mkfs(int fd);
int setup_sb(int fd, __uint32_t cblocks);
int setup_bbm(int fd, __uint32_t bused, __uint32_t cblocks);
int setup_ibm(int fd, __uint32_t cinodes, __uint32_t cblocks);

int main(int argc, const char **argv) {
	if (argc == 1)  {
		panic("no device specified");
	}

	int fd;

	char *dev = malloc(strlen(argv[1]) + 1);
	checkp(dev);
	strcpy(dev, argv[1]);
	
	fd = open(dev, O_RDWR);
	/* a fd can be 0, but fd-0 already in use, so `> 0` works */
	assert(fd > 0);
	mkfs(fd);

	free(dev);
	close(fd);

	return 0;
}




int mkfs(int fd) {
	__uint64_t dsize = lseek(fd, 0, SEEK_END);
	__uint64_t cblocks = dsize / BLOCK_SIZE;
	__uint64_t bused;
	lseek(fd, 0, SEEK_SET);

	bused = setup_sb(fd, cblocks);
	setup_bbm(fd, bused, cblocks);

#define INODES_COUNT (((cblocks * BLOCK_SIZE) >> 9) / sizeof(struct inode))

	setup_ibm(fd, INODES_COUNT, cblocks);

	return 0;

#undef INODES_COUNT
}


int setup_sb(int fd, __uint32_t cblocks) {
	superblock_d.magic = SB_MAGIC;
	superblock_d.cblocks = cblocks;

	printf("blocks count: %u\n", cblocks);

	/* block bitmap start at second block */
	/* `>> 3` is faster than ` / 8` */

	/* MINODES_USED = 4GB * 64 = 256GB */
	/* if you want use 4G inodes, you must have a disk with 128TB (MINODES_USED * 512) */

	/* don't forget `ALIGNUP_4096`! else maybe overflow, I did it :-( */

#define BBM_START (0 + 1)
#define BBM_BLOCKS (ALIGNUP_4096(cblocks >> 3))


	/* on my computer, a 64GB disk will have 2 million inodes, but it takes about only 50000 */

#define INODES_COUNT (((cblocks * BLOCK_SIZE) >> 9) / sizeof(struct inode))

#define IBM_START (BBM_START + BBM_BLOCKS + 1)
#define IBM_BLOCKS (ALIGNUP_4096(INODES_COUNT >> 3))

#define INODES_START (IBM_START + IBM_START + 1)
#define INODES_BLOCKS (ALIGNUP_4096(INODES_COUNT * sizeof(struct inode)))

#define FIRST_DATA_BLOCK (INODES_START + INODES_BLOCKS + 1)
	
	superblock_d.block_bitmap = BBM_START;

	/* inodes size : disk size = 1 : 511 */
	/* so, if you use a 64G disk, you have about 2 million inodes */

	/* ` >> 9` is faster than ` / 512` */
	/* no anyone have a 128TB disk (maybe future), so I didn't check (cinodes > MAX_INODES) */
	superblock_d.cinodes = INODES_COUNT;
	superblock_d.first_datablock = FIRST_DATA_BLOCK;

	printf("inodes count: %u\n", INODES_COUNT);
	printf("first data block: %u\n", FIRST_DATA_BLOCK);

	/* write to first block */
	bpwrite(fd, (char *) &superblock_d, BLOCK_SIZE, 0);

	return FIRST_DATA_BLOCK - 1;
	
#undef BBM_START
#undef BBM_BLOCKS

#undef INODES_START
#undef INODES_BLOCKS

#undef IBM_START
#undef IBM_BLOCKS

#undef INODES_COUNT

#undef FIRST_DATA_BLOCK
}


int setup_bbm(int fd, __uint32_t bused, __uint32_t cblocks) {
	__uint64_t i;

#define BBM_START (0 + 1)
#define BBM_BLOCKS (ALIGNUP_4096(cblocks >> 3 >> 9))

	__uint8_t *bbm = (__uint8_t *) malloc(BBM_BLOCKS * BLOCK_SIZE);
	checkp(bbm);

	cfs.block_bitmap = bbm;

	/* bused: block used. if bused == 1, one block used (0) */
	for (i = 0; i < bused; i++) 
		bitmap_set(bbm, i, 1);

	bpwrite(fd, bbm, BBM_BLOCKS * BLOCK_SIZE, BBM_START);

	return 0;
}


int setup_ibm(int fd, __uint32_t cinodes, __uint32_t cblocks) {
	/* find two free inodes (0 and 1), and write to it */
	/* but first, we need load inodes bitmap */

#define BBM_START (0 + 1)
#define BBM_BLOCKS (ALIGNUP_4096(cblocks >> 3 >> 9))

#define IBM_START (BBM_START + BBM_BLOCKS + 1)
#define IBM_BLOCKS (ALIGNUP_4096(cinodes >> 3))

#define INODES_START (IBM_START + IBM_BLOCKS + 1)
#define INODES_BLOCKS (ALIGNUP_4096(cinodes * sizeof(struct inode)))


	__uint8_t *ibm = (__uint8_t *) malloc(IBM_BLOCKS * BLOCK_SIZE);
	checkp(ibm);

	cfs.inode_bitmap = ibm;

	bitmap_set(ibm, 0, 1);
	bitmap_set(ibm, 1, 1);

	/* and fill inodes `.` and `..` */
	struct inode inode;
	
	inode.inode = 0;
	inode.pinode = 0;

	inode.mode = IMD_AP_ROOT_RD | 
			IMD_AP_ROOT_WR | 
			IMD_AP_ROOT_XR | 
			IMD_AP_USER_RD | 
			IMD_AP_USER_WR | 
			IMD_AP_USER_XR | 
			IMD_FT_FOLDER | 
			IMD_FT_POINTER;

	inode.block_count = 0;
	inode.boffset = 0;
	memset(inode.blocks, 0, 8 * 4);
	memset(inode.reserved, 0, 16);

	struct inode inode_parent;

	inode_parent.inode = 1;
	inode_parent.pinode = 0;

	inode_parent.mode = IMD_AP_ROOT_RD | 
				IMD_AP_ROOT_WR | 
				IMD_AP_ROOT_XR | 
				IMD_AP_USER_RD | 
				IMD_AP_USER_WR | 
				IMD_AP_USER_XR | 
				IMD_FT_FOLDER | 
				IMD_FT_POINTER;

	inode_parent.block_count = 0;
	inode_parent.boffset = 0;
	memset(inode_parent.blocks, 0, 8 * 4);
	memset(inode.reserved, 0, 16);

	char inodes[(sizeof(struct inode) * 2)];
	memcpy(inodes, (char *) &inode, sizeof(struct inode));
	memcpy(inodes + (sizeof(struct inode)), (char *) &inode_parent, sizeof(struct inode));
	bpwrite(fd, inodes, sizeof(struct inode) * 2, INODES_START);

	return 0;
}


