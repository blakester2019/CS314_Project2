#include "fs.h"

unsigned char* fs;

void mapfs(int fd)
{
  if ((fs = mmap(NULL, FSSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL)
  {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }
}


void unmapfs()
{
  munmap(fs, FSSIZE);
}


void formatfs()
{

}


void loadfs()
{

}


void lsfs()
{
  
}

void addfilefs(char* fname)
{

}


void removefilefs(char* fname)
{

}


void extractfilefs(char* fname)
{

}