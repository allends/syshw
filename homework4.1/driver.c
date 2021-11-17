#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

void printadd(void* ptr){
  printf("the address is %p\n", (int) ptr);
}

int main(int argc, char** argv){
  myinit(0);
  dump_heap();

  char* test = alloc(5);
  strcpy(test, "hell");

  char* second = alloc(9);
  strcpy(second, "longer");
  dump_heap();

  myfree(test);

  // try to reallocate that first one

  char* another = alloc(4);
  strcpy(another, "dff");

  printadd(another);

  dump_heap();

  mycleanup();
  return 0; 
}