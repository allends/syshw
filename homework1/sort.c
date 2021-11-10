#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void selectionSortStrings(char* arr[], int n)
{
    int i, j;
    int min_idx;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (strcmp(arr[j], arr[min_idx]) < 0)
                min_idx = j;
 
        // Swap the found minimum element
        // with the first element
        if(i!=min_idx){
          char *temp = (char *)malloc(sizeof(char[25]));
          strcpy(temp, arr[i]);
          strcpy(arr[i], arr[min_idx]);
          strcpy(arr[min_idx], temp);
          free(temp);
        }
    }
}

void selectionSortNumbers(int arr[], int n)
{
    int i, j, min_idx;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;
        int temp = arr[min_idx];
        arr[min_idx] = arr[i];
        arr[i] = temp;
    }
}

int lexSort(){
  char input[25];
  int index = 0;
  int size = 10;
  char **values = (char**)malloc(10 * sizeof(char*));
  for(int i=0; i< 10; i++ ){ 
    values[i] = malloc(sizeof(char[25]));
  }
  while(scanf(" %s", input) == 1){
    if(index >= size-1){
      size = size * 2;
      values = (char**)realloc(values, size * sizeof(char*));
      for(int i=index; i<size; i++ ){ 
        values[i] = malloc(sizeof(char[25]));
      }
    }
    strcpy(values[index], input);
    index++;
  }
  // sort the array here
  selectionSortStrings(values, index);

  for(int i=0; i<index; i++){
    printf("%s\n", values[i]);
  }
  free(values);
  return 0;
}

int numSort(){
  int* input = malloc(sizeof(int));
  int index = 0;
  int size = 10;
  int *values = (int*)malloc(size * sizeof(int));
  while(scanf(" %d", input) == 1){
    if(index >= size-1){
      size = size * 2;
      values = (int*)realloc(values, size * sizeof(int));
    }
    values[index] = *input;
    index++;
  }
  // sort the array here
  selectionSortNumbers(values, index);

  for(int i=0; i<index; i++){
    printf("%d\n", values[i]);
  }
  free(values);
  return 0;
}

int main(int argc, char **argv){
  int numFlag = strcmp(argv[argc-1], "-n") == 0;
  printf("numFlag: %d \n", numFlag);
  if(numFlag){
    numSort();
  }else{
    lexSort();
  }
  return 0;
}