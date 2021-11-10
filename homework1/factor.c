#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
  int input = atoi(argv[1]);
  int i = 2;
  while(input != 1 && i<=input){
    if(input%i==0){
      printf("%d ", i);
      input = input / i;
      i = 2;
    }else{
      i++;
    }
  }
  return 0;
}
