#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

void printadd(void* ptr){
  printf("the address is %d\n", ptr);
}

int main(int argc, char** argv){
  myinit(0);
  char* test = alloc(10);
  dump_heap();

  char* another = alloc(3);
  dump_heap();

  char* another2 = alloc(3);
  dump_heap();

  mycleanup();
  return 0; 
}