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
void* myrealloc(void* ptr, size_t size);
void dump_heap();
void splitseg(metadata_t* ptr, size_t size);

#define INIT_MALLOC 1000000

int alogrithm = 0;

metadata_t* heap;
int headersize = sizeof(metadata_t);

void myinit(int allocAlg){
  alogrithm = allocAlg;
  heap = (metadata_t *) calloc( 1, INIT_MALLOC);
  heap->size = INIT_MALLOC - headersize;
  heap->taken = false;
  heap->next = NULL; 
  printf("tag is %d bytes\n", (int) headersize);
}

void* alloc(size_t size){

  // TODO: have a switch that call the correct algo for the algorithm value

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
      if(curr->taken == false){
        printf("error: already freed\n");
      }
      curr->taken = false;
      memset(curr + headersize, 0, curr->size);
      coalesce();
      // curr->size = roundUp(curr->size);
      return;
    }
    curr = curr->next;
  }
  printf("error: not a malloced address\n");
  return;
}

void* myrealloc(void* ptr, size_t size){
  // first find the pointer that we want with a linear search

  if(size == 0 && ptr == NULL){
    return NULL;
  }

  if(size == 0){
    myfree(ptr);
    return NULL;
  }

  if(ptr == NULL){
    return malloc(size);
  }

  metadata_t* curr = heap;
  while(curr){
    if(curr + headersize == ptr){
      // we found the correct ptr to realloc
      if(roundUp(curr->size) >= size){
        curr->size = size;
        return curr + headersize;
      }
      int sizetocopy = curr->size;
      void* newspot = alloc(size);
      memcpy(newspot, curr + headersize, sizetocopy);
      myfree(curr + headersize);
      return newspot;
    }
    curr = curr->next;
  }
}

void splitseg(metadata_t* ptr, size_t size){
  int blocksize = roundUp(size);
  // we need more sophisticated way to determine when to split
  int room = ptr->size - blocksize;
  if(room < headersize){
    ptr->size = size;
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

void coalesce(){
    metadata_t* curr = heap;
    metadata_t* checkahead;
    int size = 0;
    while(curr){
      if(curr->taken == false){
        checkahead = curr->next;
        size = roundUp(curr->size);
        while(checkahead && checkahead->taken == false){
          size = size + roundUp(checkahead->size) + headersize; // full size of the unallocated block that we can free

          checkahead = checkahead->next;
          // checkahead will have what we should point to next when it finishes
        }
        // remove the old headers and update the size
        curr->next = checkahead;
        curr->size = size;
      }

      curr = curr->next;
    }
    printf("coalescing\n");
    return;
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
  memset(heap, 0, INIT_MALLOC);
  free(heap);
}


