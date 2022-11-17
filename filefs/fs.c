#include "fs.h"

unsigned char* fs;
struct superBlock* superBlock;
unsigned int* bitmap[MAX_BLOCKS];
struct inode* inodesBlock[MAX_INODES];
struct directory rootDirectory[MAX_DIR_FILES];


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
 *  fname: name of the of the filesystems file
 * 
 */
void formatfs(char* fname){
  //write superblock
  FILE* fsFile = fopen(fname, "w+");
  initSuperBlock(superBlock);
  fwrite(superBlock, sizeof(struct superBlock), 1, fsFile);
  //write bitmap
  initBitmap(bitmap);
  fwrite(bitmap, (sizeof(unsigned int) * MAX_BLOCKS), 1, fsFile);
  //write inodeblock
  fwrite(inodesBlock, (sizeof(struct inode) * MAX_INODES), 1, fsFile);
  //write root directory
  fwrite(&rootDirectory, (sizeof(struct directory) * MAX_DIR_FILES), 1, fsFile);
  fclose(fsFile);
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
void loadfs(char* fname){
  FILE* fsFile = fopen(fname, "r");
  fread(superBlock, sizeof(struct superBlock), 1, fsFile);
  for(int i = 0; i < MAX_BLOCKS; i++) {
    fread(bitmap[i], sizeof(unsigned int), 1, fsFile);
  }
  for(int i = 0; i < MAX_INODES; i++) {
    fread(inodesBlock[i], sizeof(struct inode), 1, fsFile);
  }
  fclose(fsFile);
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
  superBlock->numBlocks = MAX_BLOCKS * 10;
  superBlock->blockSize = 512; //prolly need to make const for this
  superBlock->numInodes = MAX_INODES;
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

void initBitmap(unsigned int* bitmap[]) {
  for(int i = 0; i < MAX_BLOCKS; i++) {
    bitmap[i] = 0;
  }
}