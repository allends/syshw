#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

typedef struct metadata {
  bool taken;
  size_t size;
  struct metadata* next;
} metadata_t;

void myinit(int allocAlg);
void myfree(void* ptr);
void dump_heap();
void splitseg(metadata_t* ptr, size_t size);

#define INIT_MALLOC 1000000

metadata_t* heap;
int headersize = sizeof(metadata_t);

void myinit(int allocAlg){
  heap = (metadata_t *) calloc( 1, INIT_MALLOC);
  heap->size = INIT_MALLOC - headersize;
  heap->taken = false;
  heap->next = NULL; 
  printf("tag is %d bytes\n", (int) headersize);
}

void* alloc(size_t size){
  metadata_t* curr = heap;

  // loop to find a fit that is free
  while(curr){
    if(curr->size >= size && curr->taken == false){
      // we found a match, so return it
      // split it if neccesarry
      splitseg(curr, size);
      curr->taken = true;
      return curr + headersize;
    }
    curr = curr->next;
  }
}

void myfree(void* ptr){
  metadata_t* curr = heap;
  // we are going to look linearly for the correct header;
  while(curr){
    if(curr + headersize == ptr){
      curr->taken = false;
      // curr->size = roundUp(curr->size);
      return;
    }
    curr = curr->next;
  }
  return;
}

void splitseg(metadata_t* ptr, size_t size){
  int blocksize = roundUp(size);
  // we need more sophisticated way to determine when to split
  int room = ptr->size - blocksize;
  if(room < headersize){
    return;
  }
  // start of the next header should be at ptr + 24 + blocksize
  metadata_t* newheader = ptr + headersize + blocksize;
  newheader->size = ptr->size - blocksize - headersize;
  newheader->taken = false;
  newheader->next = ptr->next;

  ptr->size = size;
  ptr->next = newheader;
  return;
}

int roundUp(int numToRound){
  int multiple = 8;
    if (multiple == 0)
        return numToRound;

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}

void dump_heap(){
  metadata_t* curr = heap;
  while(curr){
    printf("address: %p \t size: %d \t allocated: %d value: %s\n", curr + headersize, (int) curr->size, (int) curr->taken, curr + headersize);
    curr = curr->next;
  }
  printf("--------------------------------\n\n");
}

void mycleanup(){
  free(heap);
}


