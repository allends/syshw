#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

void myinit(int allocAlg);
void myfree(void* ptr);
void dump_heap();

#define INIT_MALLOC 1000000

typedef struct metadata {
  size_t size;
  struct metadata* next;
} metadata_t;

const int shift = 30;
int taken = 0b1 << shift;

metadata_t* heap;

void myinit(int allocAlg){
  heap = (metadata_t *) calloc( 1, INIT_MALLOC);
  heap->size = INIT_MALLOC | taken;
  heap->next = NULL; 
  printf("tag is %d bytes\n", sizeof(metadata_t));
}

void* alloc(size_t size){
  
  // traverse the headers of the list
  metadata_t* curr = heap;
  while(curr){
    int curr_size = curr->size & ~taken;
    int taken = curr->size & taken >> shift;
    printf("size is %d\n", curr_size);
    if(curr_size >= size && !taken){
      printf("a match is found\n");

      // distance to next block
      double bytes = (double) size;
      double address_interval = 8.0;
      int whole_blocks = (int) ceil(bytes / address_interval);

      metadata_t* next = curr + 8*whole_blocks;

      int new_size = curr_size - whole_blocks*8 - 16;
      next->next = NULL;
      next->size = new_size;

      curr->size = size;
      curr->next = next;
      return curr + 16;
    }
    curr = curr->next;
  }

}

void myfree(void* ptr){
  
}

void dump_heap(){
  metadata_t* curr = heap;
  while(curr){
    printf("address: %d size: %d \t", curr, curr->size & ~taken);
    if(curr->size & taken >> shift == 1){
      printf("%s\n", curr + 16);
    }else{
      printf("\n");
    }
    curr = curr->next;
  }
  printf("\n");
}


