#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

void printadd(void* ptr){
  printf("the address is %p\n", (int) ptr);
}

int main(int argc, char** argv){
  myinit(2);
  dump_heap();

  char* test = alloc(5);
  strcpy(test, "test");

  char* second = alloc(9);
  strcpy(second, "second");
  dump_heap();

  alloc(7);

  test = myrealloc(test, 9);
  dump_heap();
  char* takefirst = alloc(7);
  strcpy(takefirst, "tkf");
  dump_heap();

  myfree(test);
  dump_heap();

  // we should expect the first two blocks to be combined now!
  myfree(second);

  dump_heap();
  mycleanup();
  return 0; 
}