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
typedef struct Directory
{
  char name[255];
  char parentDirectory[255];
} Directory;

typedef struct File
{
  char name[255];
  int inode_id;
  char parentDirectory[255];
} File;

typedef struct SuperBlock
{
  int currInode;
} SuperBlock;

typedef struct FBL
{
  int blocks[10000];
} FBL;

typedef struct inode
{
  int id;
  char name[255];
  int InUse;
  char type[3]; // "REG" or "DIR"
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

// Struct Declarations
SuperBlock SB;
FBL fbl;
inode inodes[100];
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
    // Find First Inode not in use
    // find size of c.txt to determine how many blocks --> UPPER_BOUND(size / 512)
    // Find blocks not in use from BFL
    // Populate blocks and add block index to inode struct
  }

  if (remove)
  {
    // Find directory containing toremove
    // Find associated inode (do not remove yet)
    // Clear data blocks and flip the blocks to free in the FBL
    // Set inode to not in use
    // Also keep in mind we need to remove any directories which do not
    // contain a "regular" file (I think we need to do this)
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
  Data t_data;

  FILE* fp = fopen(fsname, "a");

  if (!fp)
  {
    printf("Could not open file\n");
    exit(-1);
  }

  t_SB.currInode = 9;
  fwrite(&t_SB, sizeof(SuperBlock), 1, fp);
  
  for (int i = 0; i < 10000; i++)
  {
    t_fbl.blocks[i] = 0;
  }
  
  for (int i = 0; i < 100; i++)
  {
    t_inodes[i].InUse = 0;
    for (int j = 0; j < 100; j++)
    {
      t_inodes[i].blocks[j] = -1;
    }
  }
  

  fwrite(&t_fbl, sizeof(t_fbl), 1, fp);
  fwrite(&t_inodes, sizeof(inode), 100, fp);
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
  fread(&data, sizeof(DataBlock), 10000, fp);
  fclose(fp);
}