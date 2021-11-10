# mymalloc project 


## overview 

we have to make the following functions

void myinit(int allocAlg);
void* mymalloc(size_t size);
	this is giving me an error and i do not know why
	calling this any other name fixes the issue


void myfree(void* ptr);
void* myrealloc(void* ptr, size_t size);
void mycleanup();
double utilization();

## structure of an allocated block
header (contains the following):
  size of the block (4 bytes)
  if its free (1 bit)
  relative address the next block thats free (ie how much to add?)


