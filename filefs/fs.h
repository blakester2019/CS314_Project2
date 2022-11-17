#ifndef __FS_H__
#define __FS_H__
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define FSSIZE 10000000
#define MAX_FILE_BLOCKS 100
#define MAX_FILE_NAME_SIZE 255
#define MAX_BLOCKS 500 //reason this is so low is were using an unsigned int bitmap
#define MAX_INODES 500
#define MAX_DIR_FILES 100


extern unsigned char* fs;

struct superBlock { // might add locations of inode blocks, bitmap, and root dir too :)
    int numBlocks;
    int blockSize;
    int numInodes;
    // int inodeBlockStart;
    // int bitmapStart;
    // int rootDirStart;
};

struct inode {
    int size;
    bool isDir;
    int blocks[MAX_FILE_BLOCKS];
};

struct directory {
    char fileName[MAX_FILE_NAME_SIZE];
    int inodeNum;
};

struct dataBlock {
    int randomJunk;
};

void mapfs(int fd);
void unmapfs();
void formatfs();
void loadfs();
void lsfs();
void addfilefs(char* fname);
void removefilefs(char* fname);
void extractfilefs(char* fname);

/* Max's added functions */
void initSuperBlock(struct superBlock* superBlock);
void initBitmap(unsigned int* bitmap[]);

#endif
