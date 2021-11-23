#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mymalloc.h"

#define OP_COUNT 1000
#define POINTERS 10

void utilization();

void printadd(void* ptr){
  printf("the address is %p\n", (int) ptr);
}

int main(int argc, char** argv){
  myinit(2);
  dump_heap();

  // char* vars[5];
  // for(int i=0; i<5; i++){
  //   vars[i] = alloc(i);
  //   dump_heap();
  // }
  // for(int i=0; i<5; i++){
  //   myfree(vars[i]);
  //   dump_heap();
  // }

  utilization();
  mycleanup();
  return 0; 
}

void utilization(){
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long start = 1000000 * tv.tv_sec + tv.tv_usec;

  srand(time(NULL));

  char* variables[POINTERS];
  int operations_completed;

  for(int i=0; i< POINTERS; i++){
    variables[i] = alloc(rand_size());
    operations_completed++;
  }
  while(operations_completed < OP_COUNT){
    for(int i=0; i< POINTERS; i++){
      if(variables[i] = realloc(variables[i], rand_size()) == NULL){
        variables[i] = alloc(rand_size());
      }else{
        free(variables[i]);
      }
      variables[i] = alloc(rand_size());
      operations_completed++;
    }
  }
  dump_heap();
  for(int i = 0; i < OP_COUNT; i++){
    myfree(variables[i]);
    operations_completed++;
  }

  gettimeofday(&tv, NULL);
  unsigned long end = 1000000 * tv.tv_sec + tv.tv_usec;
  unsigned long elapsed_time = end - start;
  printf("the elapsed time was %ld microseconds\n", elapsed_time);
  double elapsed_seconds = elapsed_time / 1000000;
  double ops_per_second = OP_COUNT / elapsed_seconds;
  printf("Throughput: %f\n", ops_per_second);
  return;
}

int rand_size(){
  int block = rand() % 255 + 1;
  return block;
}