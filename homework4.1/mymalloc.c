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

const int shift = sizeof(int)*8 - 1;
int taken = 0b1 << shift;

metadata_t* heap;

void myinit(int allocAlg){
  heap = (metadata_t *) calloc( 1, INIT_MALLOC);
  heap->size = INIT_MALLOC | taken;
  heap->next = NULL; 
  printf("tag is %d bytes\n", sizeof(metadata_t));
}

void* alloc(size_t size){
  printf("ALLOC HAS BEEN CALLED WITH SIZE %d\n", size);
  // traverse the headers of the list
  metadata_t* curr = heap;
  while(curr){
    int curr_size = curr->size & ~taken;
    int taken_bit = (curr->size & taken) >> shift;
    printf("The size of the current block that we are looking at is %d and the taken bit is %d\n", curr_size, taken_bit);
    if(curr_size >= size && !taken_bit){
      printf("SPOT FOUND AT ADDRESS %d\n", curr + 16);

      // distance to next block
      if(curr->next == NULL){
        double bytes = (double) size;
        double address_interval = 8.0;
        int whole_blocks = (int) ceil(bytes / address_interval);

        metadata_t* next = curr + 8*whole_blocks;

        int new_size = curr_size - whole_blocks*8 - 16;
        next->next = NULL;
        next->size = new_size;
        curr->size = size | taken;
        curr->next = next;
        return curr + 16;
      }
      curr->size = size | taken;
      // we do not touch curr->next since it is already defined here
      return curr + 16;
    }
    curr = curr->next;
  }

}

void myfree(void* ptr){
  metadata_t* curr = heap;
  metadata_t* last = NULL;
  while(curr){
    if(curr + 16 == ptr){
      printf("target has been found!\n");
      if(last){
        // last->next = curr->next; // this is only for an explicity linked list
      }
      curr->size = curr->size & ~taken;
      return;
    }
    last = curr;
    curr = curr->next;
  }
  return;
}

void dump_heap(){
  metadata_t* curr = heap;
  while(curr){
    printf("address: %p size: %d \t", curr + 16, curr->size & ~taken);
    if(curr->size & taken >> shift == 1){
      printf("%s\n", curr + 16);
    }else{
      printf("unallocated \n");
    }
    curr = curr->next;
  }
  printf("\n");
}

void mycleanup(){
  free(heap);
}


