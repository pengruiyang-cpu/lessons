#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>

#include "cfs.h"
#include "bitmap.h"


#define panic(s) \
	printf("panic: "s"\n"); \
	exit(1);

#define bpwrite(fd, buffer, size, bn) (pwrite(fd, buffer, size, bn * BLOCK_SIZE))
#define bpread(fd, buffer, size, bn) (pread(fd, buffer, size, bn * BLOCK_SIZE))


int mount_cfs(int fd);
int umount_cfs(int fd);

__uint32_t read_inode(int fd, __uint32_t inode, struct inode *buffer);
__uint32_t write_inode(int fd, __uint32_t inode, struct inode *buffer);

__uint32_t balloc(void);
__uint32_t ialloc(void);

__uint32_t inode_bmap(int fd, struct inode *inode, __uint32_t bn, int create);

int ireadb(int fd, struct inode *inode, __uint32_t bn, char *buffer);
int iwriteb(int fd, struct inode *inode, __uint32_t bn, char *buffer);


int main(int argc, char **argv) {
	if (argc == 1) { 
		panic("no device specified");
	}

	int fd;
	char *dev = malloc(strlen(argv[1]) + 1);
	checkp(dev);
	strcpy(dev, argv[1]);

	fd = open(dev, O_RDWR);
	assert(fd > 0);

	mount_cfs(fd);

	struct inode first_inode;
	read_inode(fd, 0, &first_inode);

	printf("inode 0.inode = %u\n", first_inode.inode);

	char buffer[4096] = {"hello, world. hello, world. hello, world. HELLO< WORLD> "};

	__uint32_t bno = inode_bmap(fd, &first_inode, 0, 1);
	printf("bmap returns bno: %u\n", bno);
	bpwrite(fd, buffer, BLOCK_SIZE, bno);

	write_inode(fd, 0, &first_inode);

	umount_cfs(fd);

	free(dev);
	close(fd);

	return 0;
}


int mount_cfs(int fd) {
	bpread(fd, (char *) &superblock_d, BLOCK_SIZE, 0);

	if (superblock_d.magic != SB_MAGIC) {
		panic("super block magic error (not a CFS)\n");
	}

	printf("blocks count: %u\n", superblock_d.cblocks);
	printf("blocks bitmap: %u\n", superblock_d.block_bitmap);
	printf("inodes bitmap: %u\n", superblock_d.inode_bitmap);
	printf("inodes address: %u\n", superblock_d.inodes);
	printf("inodes count: %u\n", superblock_d.cinodes);
	printf("first data block: %u\n", superblock_d.first_datablock);

	cfs.superblock = &superblock_d;
	cfs.block_bitmap = malloc(superblock_d.bbm_blocks * BLOCK_SIZE);

	if (!cfs.block_bitmap) 
		return 1;

	bpread(fd, cfs.block_bitmap, superblock_d.bbm_blocks * BLOCK_SIZE, superblock_d.block_bitmap);

	cfs.inode_bitmap = malloc(superblock_d.ibm_blocks * BLOCK_SIZE);

	if (!cfs.inode_bitmap) {
		free(cfs.block_bitmap);
		return 1;
	}

	bpread(fd, cfs.inode_bitmap, superblock_d.ibm_blocks * BLOCK_SIZE, superblock_d.inode_bitmap);

	return 0;
}


int umount_cfs(int fd) {
	bpwrite(fd, (char *) &superblock_d, BLOCK_SIZE, 0);

	bpwrite(fd, cfs.block_bitmap, BLOCK_SIZE, superblock_d.block_bitmap);
	free(cfs.block_bitmap);

	bpwrite(fd, cfs.inode_bitmap, BLOCK_SIZE, superblock_d.inode_bitmap);
	free(cfs.inode_bitmap);

	return 0;
}


__uint32_t read_inode(int fd, __uint32_t inode, struct inode *buffer) {
	__uint32_t block = superblock_d.inodes + (inode >> INODES_PER_BLOCK_SHIFT);
	struct inode inodes[INODES_PER_BLOCK];

	bpread(fd, (char *) inodes, sizeof(struct inode) * INODES_PER_BLOCK, block);

	/* buffer = (struct inode *) inodes + (inode & INODES_PER_BLOCK_MASK); */
	memcpy(buffer, (struct inode *) inodes + (inode & INODES_PER_BLOCK_MASK), sizeof(struct inode));

	return inode;
}


__uint32_t write_inode(int fd, __uint32_t inode, struct inode *buffer) {
	__uint32_t block = superblock_d.inodes + (inode >> INODES_PER_BLOCK_SHIFT);
	struct inode inodes[INODES_PER_BLOCK];

	bpread(fd, (char *) inodes, sizeof(struct inode) * INODES_PER_BLOCK, block);
	memcpy((struct inode *) inodes + (inode & INODES_PER_BLOCK_MASK), buffer, sizeof(struct inode));
	bpwrite(fd, (char *) inodes, sizeof(struct inode) * INODES_PER_BLOCK, block);

	return inode;
}


__uint32_t balloc(void) {
	__uint32_t block;

	for (block = 0; bitmap_read(cfs.block_bitmap, block); block++) {
		if (block == superblock_d.cblocks) 
			return 0;
	}

	bitmap_set(cfs.block_bitmap, block, 1);

	return block;
}



__uint32_t ialloc(void) {
	__uint32_t inode;

	for (inode = 0; bitmap_read(cfs.inode_bitmap, inode); inode++) {
		if (inode == superblock_d.cinodes) 
			return 0;
	}

	bitmap_set(cfs.inode_bitmap, inode, 1);

	return inode;
}


__uint32_t inode_bmap(int fd, struct inode *inode, __uint32_t bn, int create) {
	if (!inode) 
		return 0;

	__uint32_t i;

	if (bn > INODE_MAX_BLOCKS) 
		return 0;

	if (bn < INODE_NIND_BLOCKS) {
		if (!inode->blocks[bn] && create) 
			inode->blocks[bn] = balloc();

		if (!inode->blocks[bn]) 
			return 0;

		return inode->blocks[bn];
	}

	bn -= INODE_NIND_BLOCKS;

	if (bn < INODES_PER_BLOCK) {
		i = inode->blocks[INODE_IND_BLOCK];
		if (!i && create) 
			i = inode->blocks[INODE_IND_BLOCK] = balloc();

		if (!i) 
			return 0;

		__uint32_t blocks[BLOCK_SIZE / 4];
		bpread(fd, blocks, BLOCK_SIZE, i);

		if (!blocks[bn] && create) 
			blocks[bn] = balloc();

		if (!blocks[bn]) 
			return 0;

		return blocks[bn];
	}

	bn -= INODES_PER_BLOCK;

	if (bn < INODES_PER_BLOCK * INODES_PER_BLOCK) {
		i = inode->blocks[INODE_DIND_BLOCK];

		if (!i && create)
			i = inode->blocks[INODE_DIND_BLOCK] = balloc();

		if (!i) 
			return 0;

		__uint32_t blocks[BLOCK_SIZE / 4];
		bpread(fd, blocks, BLOCK_SIZE, i);
		i = blocks[bn / INODES_PER_BLOCK];

		if (!i && create) 
			i = blocks[bn / INODES_PER_BLOCK] = balloc();

		if (!i) 
			return 0;

		bpread(fd, blocks, BLOCK_SIZE, i);
		i = blocks[bn & INODES_PER_BLOCK_MASK];
		if (!i && create) 
			i = balloc();

		if (!i) 
			return 0;

		return i;
	}

	bn -= INODES_PER_BLOCK * INODES_PER_BLOCK;

	i = inode->blocks[INODE_TIND_BLOCK];
	if (!i && create) 
		i = inode->blocks[INODE_TIND_BLOCK] = balloc();

	if (!i) 
		return 0;

	__uint32_t blocks[BLOCK_SIZE / 4];
	bpread(fd, blocks, BLOCK_SIZE, i);
	i = blocks[bn / INODES_PER_BLOCK / INODES_PER_BLOCK];
	
	if (!i && create) 
		i = blocks[bn / INODES_PER_BLOCK / INODES_PER_BLOCK] = balloc();

	if (!i) 
		return 0;

	bpread(fd, blocks, BLOCK_SIZE, i);
	i = blocks[(bn / INODES_PER_BLOCK) & INODES_PER_BLOCK_MASK];

	if (!i && create) 
		i = blocks[(bn / INODES_PER_BLOCK) & INODES_PER_BLOCK_MASK] = balloc();

	bpread(fd, blocks, BLOCK_SIZE, i);
	i = blocks[bn & INODES_PER_BLOCK_MASK];

	if (!i && create) 
		i = balloc();

	if (!i) 
		return 0;

	return i;
}



int ireadb(int fd, struct inode *inode, __uint32_t bn, char *buffer) {
	__uint32_t block = inode_bmap(fd, inode, bn, 0);

	if (!block) 
		return 1;

	bpread(fd, buffer, BLOCK_SIZE, block);

	return 0;
}


int iwriteb(int fd, struct inode *inode, __uint32_t bn, char *buffer) {
	__uint32_t block = inode_bmap(fd, inode, bn, 1);

	if (!block) 
		return 1;

	bpwrite(fd, buffer, BLOCK_SIZE, block);
	inode->block_count++;
	get_time(inode->vtime_h, inode->vtime_l);

	return 0;
}
