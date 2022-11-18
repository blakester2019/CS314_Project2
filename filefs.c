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

// Prototypes
int zerosize(int fd);
void exitusage(char* pname);
void formatNewFS(char* fsname);
void loadFileSystem(char* fsname);
int findFBLIndex(int start, int end);
int findInodeIndex();
int fileExists(char* filename);

// Struct Declarations
SuperBlock SB;
FBL fbl;
inode inodes[100];
Directories dirs;
Data data;


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
  if ((fd = open(fsname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
  {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  else
  {
    // Check if it is a new file or not
    if (zerosize(fd))
      newfs = 1;
    
    // Create the 10MB fs file if it is new
    if (newfs)
    {
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
    
    for (int i = 0; i < 50; i++)
    {
      tokenArray[i] = malloc(255);
      strcpy(tokenArray[i], "NULL");
    }
    
    int counter = 0;
    token = strtok(toadd, "/\\");
    while (token != NULL)
    {
      strcpy(tokenArray[counter], token);
      counter++;
      token = strtok(NULL, "/\\");
    }

    printf("File: %s\n", tokenArray[counter - 1]);

    // Insert Directories
    for (int i = 0; i < counter - 1; i++)
    {
      printf("%d\n", i);
      int fbl_index = findFBLIndex(0, 150);
      int inode_index = findInodeIndex();
      // New File Object For Directories
      File newDirectory;
      strcpy(newDirectory.name, tokenArray[i]);
      newDirectory.inode_id = inode_index;
      // Update inode
      inodes[inode_index].IsDir = 1;
      inodes[inode_index].size = 0;
      inodes[inode_index].blocks[0] = fbl_index;
    }
    
    // Get Stats for file
    struct stat st;
    if (!fileExists(tokenArray[counter - 1]))
    {
      printf("No file named %s\n", tokenArray[counter - 1]);
      exit(-1);
    }
    stat(tokenArray[counter - 1], &st);
    int size = st.st_size;

    // Create File Object for File
    int numBlocks = (size + (512 - 1)) / 512;
    int inode_index = findInodeIndex();
    File newFile;
    strcpy(newFile.name, tokenArray[counter - 1]);
    newFile.inode_id = inode_index;

    // Update inode
    inodes[inode_index].IsDir = 0;
    inodes[inode_index].size = size;

    // TODO: Insert data into blocks
    int block_ids[numBlocks];
    for (int i = 0; i < numBlocks; i++)
      block_ids[i] = findFBLIndex(150, 10150);
    
    // Find blocks not in use from BFL
    // Populate blocks and add block index to inode struct
    // Truncate fs file and write SB, FBL, inodes, and Data

    // Free token array
    for (int i = 0; i < 50; i++)
      free(tokenArray[i]);
  }

  if (remove)
  {
    // Find directory containing toremove
    // Find associated inode (do not remove yet)
    // Clear data blocks and flip the blocks to free in the FBL
    // Set inode to not in use
    // Also keep in mind we need to remove any directories which do not
    // contain a "regular" file (I think we need to do this)
    // Truncate fs file and write SB, FBL, inodes, and data
  }

  if (extract)
  {
    // Find inode associated with toextract
    // Read blocks from the inode 'blocks' array
    // Extract concatanation of blocks to stdout
  }

  if(list)
  {
    // Confused how to attatch file names to inodes. 
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

  if (!fp)
  {
    printf("Could not open file\n");
    exit(-1);
  }
  
  for (int i = 0; i < 10000; i++)
    t_fbl.blocks[i] = 0;
  
  for (int i = 0; i < 100; i++)
  {
    t_inodes[i].InUse = 0;
    for (int j = 0; j < 100; j++)
      t_inodes[i].blocks[j] = -1;
  }
  
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

  if (!fp)
  {
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
  for (int i = start; i < end; i++)
  {
    if (fbl.blocks[i] == 0)
    {
      fbl.blocks[i] = 1;
      return i;
    }
  }
  return -1;
}

int findInodeIndex()
{
  for (int i = 0; i < 100; i++)
  {
    if (inodes[i].InUse == 0)
    {
      inodes[i].InUse = 1;
      return i;
    }
  }
}

int fileExists(char* filename)
{
  FILE* fp = fopen(filename, "r");
  if (fp)
  {
    fclose(fp);
    return 1;
  }
  return 0;
}