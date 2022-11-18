#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

#include <errno.h>

#include <sys/mman.h>

#include "fs.h"

// File System Structs
typedef struct File
{
  char name[255];
  int inode_id;
  int parent_inode;
} File;

typedef struct Directories
{
  File files[150];
} Directories;

typedef struct SuperBlock
{
  int currInode;
} SuperBlock;

typedef struct FBL
{
  int blocks[10150];
} FBL;

typedef struct inode
{
  int id;
  int InUse;
  int IsDir;
  int size;
  int blocks[100];
} inode;

typedef struct DataBlock
{
  char data[512];
} DataBlock;

typedef struct Data
{
  DataBlock blocks[10000];
} Data;

// Struct Declarations
SuperBlock SB;
FBL fbl;
inode inodes[100];
Directories dirs;
Data data;

// Prototypes
int zerosize(int fd);
void exitusage(char* pname);
void formatNewFS(char* fsname);
void loadFileSystem(char* fsname);
int findFBLIndex(int start, int end);
int findInodeIndex();
int fileExists(char* filename);
void insertFileIntoDirs(char* fname, int fbl_index, int inode_index, int prevInode);
void updateInode(int inode_index, int IsDir, int size, int fbl_index);
void updateFileSystem(char* fsname);
int searchDirs(char* arr[], int counter);
void extractInodeBlocks(int inode_id);
int checkIfDirExists(char* fname, int parent_inode);
void printFile(char* fname, int depth);
void listFiles(int parent, int depth);
void resetInode(int inode_id);

// Dev Prototypes
void printDirs();
void printFBL(int start, int end);

// ---- MAIN ----
int main(int argc, char** argv)
{  
  int opt;
  int create = 0;
  int list = 0;
  int add = 0;
  int remove = 0;
  int extract = 0;
  char* toadd = NULL;
  char* toremove = NULL;
  char* toextract = NULL;
  char* fsname = NULL;
  int fd = -1;
  int newfs = 0;
  int filefsname = 0;
 
  // Parse Passed Options
  while ((opt = getopt(argc, argv, "la:r:e:f:")) != -1) {
    switch (opt) {
    case 'l':
      list = 1;
      break;
    case 'a':
      add = 1;
      toadd = strdup(optarg);
      break;
    case 'r':
      remove = 1;
      toremove = strdup(optarg);
      break;
    case 'e':
      extract = 1;
      toextract = strdup(optarg);
      break;
    case 'f':
      filefsname = 1;
      fsname = strdup(optarg);
      break;
    default:
      exitusage(argv[0]);
    }
  }
  
  // Throw error if no file was passed
  if (!filefsname)
    exitusage(argv[0]);

  // Open the file
  if ((fd = open(fsname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  else {
    // Check if it is a new file or not
    if (zerosize(fd))
      newfs = 1;
    
    // Create the 10MB fs file if it is new
    if (newfs) {
      formatNewFS(fsname);
      close(fd);
    }
  }
  
  loadFileSystem(fsname);

  mapfs(fd);
  
  if (add)
  {
    // Create directories such as "a/b/c.txt" (toadd)
    char* token;
    char* tokenArray[50];
    
    for (int i = 0; i < 50; i++) {
      tokenArray[i] = malloc(255);
      strcpy(tokenArray[i], "NULL");
    }
    
    int counter = 0;
    token = strtok(toadd, "/\\");
    while (token != NULL) {
      strcpy(tokenArray[counter], token);
      counter++;
      token = strtok(NULL, "/\\");
    }

    // Insert Directories
    int prevInode = 0;
    int next;
    for (int i = 0; i < counter - 1; i++) {
      if (next = checkIfDirExists(tokenArray[i], prevInode))
      {
        prevInode = next;
        continue;
      }
      int fbl_index = findFBLIndex(0, 150);
      int inode_index = findInodeIndex();
      insertFileIntoDirs(tokenArray[i], fbl_index, inode_index, prevInode);
      updateInode(inode_index, 1, 0, fbl_index);
      prevInode = inode_index;
    }

    // Check if file already exists
    next = checkIfDirExists(tokenArray[counter - 1], prevInode);
    if (next) {
      printf("This file already exists in this directory\n");
      exit(-1);
    }
    
    // Get Stats for file
    if (!fileExists(tokenArray[counter - 1])) {
      printf("No file named %s\n", tokenArray[counter - 1]);
      exit(-1);
    }

    struct stat st;
    stat(tokenArray[counter - 1], &st);
    int size = st.st_size;
    printf("New File Size: %d\n", size);

    // Create File Object for File
    int inode_index = findInodeIndex();
    int fbl_index = findFBLIndex(0, 150);
    insertFileIntoDirs(tokenArray[counter - 1], fbl_index, inode_index, prevInode);
    updateInode(inode_index, 0, size, fbl_index);

    // Insert data into blocks
    int numBlocks = (size + (512 - 1)) / 512;
    int block_ids[numBlocks];
    for (int i = 0; i < numBlocks; i++) {
      block_ids[i] = findFBLIndex(150, 10150);
      inodes[inode_index].blocks[i + 1] = block_ids[i];
    }

    // MAXS PROBLEM FROM HERE
    // Counter: How many different parts are in path (example: /a/b/c/fs.c = 4)
    // Token array would be ["a", "b", "c", "fs.c"] where tokenArray[counter - 1] is the filename
    // numBlocks - how many blocks the file needs
    // block_ids array is to show which blocks the inode is going to use (ex. [5, 77, 4, 52])
    FILE *fp = fopen(tokenArray[counter - 1], "r");
    int remaining = size;
    for (int i = 0; i < numBlocks; i++) {
      char buffer[512];
      if (remaining >= 511) {
        strcpy(buffer, "");
        fread(&buffer, 1, 511, fp);
        remaining = remaining - 511;
      }
      else {
        strcpy(buffer, "");
        fread(&buffer, 1, 1, fp);
        printf("%s\n", buffer);
        remaining = 0;
      }
      strcpy(data.blocks[block_ids[i]].data, buffer);
    }
    // DOWN TO HERE
    
    // Truncate fs file and write SB, FBL, inodes, and Data
    updateFileSystem(fsname);

    // Free token array
    for (int i = 0; i < 50; i++)
      free(tokenArray[i]);
  }

  if (remove)
  {
    // Tokenize path
    char* token;
    char* tokenArray[50];
    
    for (int i = 0; i < 50; i++) {
      tokenArray[i] = malloc(255);
      strcpy(tokenArray[i], "NULL");
    }
    
    int counter = 0;
    token = strtok(toremove, "/\\");
    while (token != NULL) {
      strcpy(tokenArray[counter], token);
      counter++;
      token = strtok(NULL, "/\\");
    }

    // Determine if file is in the path
    int file_inode = searchDirs(tokenArray, counter);
    
    if (file_inode <= 0) {
      printf("File could not be found\n");
      exit(-1);
    }

    resetInode(file_inode);
    // Also keep in mind we need to remove any directories which do not
    // contain a "regular" file (I think we need to do this)
    updateFileSystem(fsname);
  }

  if (extract)
  {
    // Find inode associated with toextract
    // Read blocks from the inode 'blocks' array
    // Extract concatanation of blocks to stdout
    char* token;
    char* tokenArray[50];
    
    for (int i = 0; i < 50; i++) {
      tokenArray[i] = malloc(255);
      strcpy(tokenArray[i], "NULL");
    }
    
    int counter = 0;
    token = strtok(toextract, "/\\");
    while (token != NULL) {
      strcpy(tokenArray[counter], token);
      counter++;
      token = strtok(NULL, "/\\");
    }
    
    int file_inode = searchDirs(tokenArray, counter);
    
    if (file_inode <= 0) {
      printf("File could not be found\n");
      exit(-1);
    }

    extractInodeBlocks(file_inode);
  }

  if(list)
  {
    listFiles(0, 0);
  }

  unmapfs();
  
  return 0;
} // end main


int zerosize(int fd)
{
  struct stat stats;
  fstat(fd, &stats);
  if(stats.st_size == 0)
    return 1;
  return 0;
}

void exitusage(char* pname)
{
  fprintf(stderr, "Usage %s [-l] [-a path] [-e path] [-r path] -f name\n", pname);
  exit(EXIT_FAILURE);
}

void formatNewFS(char* fsname)
{
  SuperBlock t_SB;
  FBL t_fbl;
  inode t_inodes[100];
  Directories t_dirs;
  Data t_data;

  FILE* fp = fopen(fsname, "a");

  if (!fp) {
    printf("Could not open file\n");
    exit(-1);
  }
  
  for (int i = 0; i < 10000; i++)
    t_fbl.blocks[i] = 0;
  
  for (int i = 0; i < 100; i++) {
    t_inodes[i].InUse = 0;
    for (int j = 0; j < 100; j++)
      t_inodes[i].blocks[j] = -1;
  }

  // Create Root Directory
  File root;
  strcpy(root.name, "root");
  root.inode_id = 0;
  root.parent_inode = -1;
  t_dirs.files[0] = root;
  t_inodes[0].InUse = 1;
  t_inodes[0].IsDir = 1;
  t_inodes[0].size = 0;
  t_fbl.blocks[0] = 1;
  t_inodes[0].blocks[0] = 0;
  
  fwrite(&t_SB, sizeof(SuperBlock), 1, fp);
  fwrite(&t_fbl, sizeof(t_fbl), 1, fp);
  fwrite(&t_inodes, sizeof(inode), 100, fp);
  fwrite(&t_dirs, sizeof(Directories), 1, fp);
  fwrite(&t_data, sizeof(Data), 1, fp);

  fseek(fp, FSSIZE, SEEK_SET);
  fputs("End Of File", fp);

  fclose(fp);
}

void loadFileSystem(char* fsname)
{
  FILE* fp = fopen(fsname, "r");

  if (!fp) {
    printf("File could not be opened for reading\n");
    exit(-1);
  }
  
  fread(&SB, sizeof(SuperBlock), 1, fp);
  fread(&fbl, sizeof(FBL), 1, fp);
  fread(&inodes, sizeof(inode), 100, fp);
  fread(&dirs, sizeof(Directories), 1, fp);
  fread(&data, sizeof(Data), 1, fp);
  fclose(fp);
}

int findFBLIndex(int start, int end)
{
  for (int i = start; i < end; i++) {
    if (fbl.blocks[i] == 0) {
      fbl.blocks[i] = 1;
      return i;
    }
  }
  return -1;
}

int findInodeIndex()
{
  for (int i = 0; i < 100; i++) {
    if (inodes[i].InUse == 0) {
      inodes[i].InUse = 1;
      return i;
    }
  }
}

int fileExists(char* filename)
{
  FILE* fp = fopen(filename, "r");
  if (fp) {
    fclose(fp);
    return 1;
  }
  return 0;
}

void insertFileIntoDirs(char* fname, int fbl_index, int inode_index, int prevInode)
{
  File newDirectory;
  strcpy(newDirectory.name, fname);
  newDirectory.inode_id = inode_index;
    if (prevInode >= 0)
      newDirectory.parent_inode = prevInode;
  dirs.files[fbl_index] = newDirectory;
}

void updateInode(int inode_index, int IsDir, int size, int fbl_index)
{
  inodes[inode_index].IsDir = IsDir;
  inodes[inode_index].size = size;
  inodes[inode_index].blocks[0] = fbl_index;
}

void updateFileSystem(char* fsname)
{
  FILE* fp = fopen("temp", "a");

  if (!fp) {
    printf("Could not open file\n");
    exit(-1);
  }
  
  fwrite(&SB, sizeof(SuperBlock), 1, fp);
  fwrite(&fbl, sizeof(FBL), 1, fp);
  fwrite(&inodes, sizeof(inode), 100, fp);
  fwrite(&dirs, sizeof(Directories), 1, fp);
  fwrite(&data, sizeof(Data), 1, fp);
  fclose(fp);

  remove(fsname);
  rename("temp", fsname);
}

int searchDirs(char* arr[], int counter)
{
  int parent_inode = 0;
  int next;
  // File lives in the root
  if (counter == 1) {
    return checkIfDirExists(arr[0], 0);
  }

  // Search file path
  for (int i = 0; i < counter - 1; i++) {
    for (int j = 0; j < 150; j++) {
      if (next = checkIfDirExists(arr[i], parent_inode)) {
        parent_inode = next;
        break;
      }
      else {
        return -1;
      }
    }
  }

  return checkIfDirExists(arr[counter - 1], parent_inode);
}

// MAXS PROBLEM FROM HERE
// Should iterate through the blocks of an inode and print the contents of the blocks in "data"
void extractInodeBlocks(int inode_id)
{
  char buffer[inodes[inode_id].size + 512];
  strcpy(buffer, "");
  char blockBuffer[512];
  int i = 0;
  int currBlock = inodes[inode_id].blocks[i];
  while (currBlock != -1)
  {
    strcpy(blockBuffer, data.blocks[currBlock].data);
    strcat(buffer, blockBuffer);
    i++;
    currBlock = inodes[inode_id].blocks[i];
  }
  puts(buffer);
}
// To here

// Checks if a directory exists. If so, returns that directories inode_id
int checkIfDirExists(char* fname, int parent_inode)
{
  for (int i = 0; i < 150; i++) {
    if (dirs.files[i].parent_inode == parent_inode && strcmp(dirs.files[i].name, fname) == 0) {
      return dirs.files[i].inode_id;
    }
  }
  return 0;
}

// Used for debugging. Prints all directories, both regular and dir type
void printDirs()
{
  for (int i = 0; i < 150; i++)
  {
    printf("-----------------\n");
    printf("Dirs[%d]: %s\n", i, dirs.files[i].name);
    printf("Inode: %d\n", dirs.files[i].inode_id);
    printf("Parent: %d\n", dirs.files[i].parent_inode);
  }
}

// Used for debugging. Prints fbl binary from a given start to given end
void printFBL(int start, int end)
{
  for (int i = start; i < end; i++) {
    printf("FBL[%d]: %d\n", i, fbl.blocks[i]);
  }
}

// Helps the listFiles function. Prints tabs for depth and filename
void printFile(char* fname, int depth)
{
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  printf(" %s\n", fname);
}

// List files will recursively parse the files in the fs
void listFiles(int parent, int depth)
{
  for (int i = 1; i < 150; i++)
  {
    if (dirs.files[i].inode_id == 0) {
      continue;
    }

    if (dirs.files[i].parent_inode == parent) {
      printFile(dirs.files[i].name, depth);
      listFiles(dirs.files[i].inode_id, depth + 1);
    }
  }
}

// Used in remove function to return an inode to its original state
void resetInode(int inode_id)
{
  for (int i = 0; i < 100; i++) {
    if (inodes[inode_id].blocks[i] != -1) {
      int block_index = inodes[inode_id].blocks[i];
      if (block_index < 150) {
        strcpy(dirs.files[block_index].name, "");
        dirs.files[block_index].inode_id = 0;
        dirs.files[block_index].parent_inode = 0;
        fbl.blocks[block_index] = 0;
        inodes[inode_id].blocks[0] = -1;
      } else {
        strcpy(data.blocks[block_index].data, "");
        inodes[inode_id].blocks[i] = -1;
      }
    }
  }
  inodes[inode_id].InUse = 0;
}