#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

void printadd(void* ptr){
  printf("the address is %d\n", ptr);
}

int main(int argc, char** argv){
  myinit(0);
  char* test = alloc(1);
  strcpy(test, "hello there");
  printadd(test);
  dump_heap();
  char* another = alloc(60);
  dump_heap();
  return 0; 
}