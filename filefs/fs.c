#include "fs.h"

unsigned char* fs;
struct superBlock* superBlock;
unsigned int** bitmap;
struct inode** inodesBlock;
struct directory rootDirectory[MAX_DIR_FILES];
struct dataBlock* DB;


void mapfs(int fd){
  if ((fs = mmap(NULL, FSSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL){
      perror("mmap failed");
      exit(EXIT_FAILURE);
  }
}


void unmapfs(){
  munmap(fs, FSSIZE);
}

/*
 * Function:   formatfs
 * --------------------
 * formats the filesystem file, adding superblock,
 *  bitmap, inodes block, and the root dir using fwrite, 
 *    + inits superblock and bitmap
 * 
 */
void formatfs(){
  struct superBlock* SB = fs;  
  initSuperBlock(SB);
  unsigned int** bm = fs + sizeof(struct superBlock);
  initBitmap(bm);
  inodesBlock = fs + sizeof(struct superBlock) + (sizeof(unsigned int) * MAX_BLOCKS);
  DB = fs + sizeof(struct superBlock) + (sizeof(unsigned int) * MAX_BLOCKS) + (sizeof(struct inode) * MAX_INODES);
}

/*
 *   Function: loadfs
 * --------------------
 *  this function reads from the filesystem file, 
 *    and puts the information into the superBlock,
 *    the inodes block, and the bitmap
 *  
 *  fname: this is the filesystem file
 *   
 *  no return value as its changing the ptrs
 */
void loadfs(){
  
}


void lsfs(){
  
}

void addfilefs(char* fname){

}


void removefilefs(char* fname){

}


void extractfilefs(char* fname){

}

/* Max's added functions*/

/*
 * Function: initSuperBlock
 * ------------------------
 *  this is only used once in order to 
 *  sure that the file has the correct
 *  information
 *
 *  superBlock: struct instance that will
 *    be written to the file
 *
 */

void initSuperBlock(struct superBlock* superBlock) {
  superBlock->numBlocks = 45;
  superBlock->blockSize = 512; //prolly need to make const for this
  superBlock->numInodes = 78;
}

/*
 * Function: initBitmap
 * --------------------
 *  sets everything to 0 so we know that 
 *   the whole array is free
 * 
 *  bitmap[]: the actual bitmap
 *  
 */

void initBitmap(unsigned int** bitmap) {
  for(int i = 0; i < MAX_BLOCKS; i++) {
    bitmap[i] = 0;
  }
}