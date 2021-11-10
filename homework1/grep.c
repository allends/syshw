#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int hasPattern(char *pattern, char *input, int iFlag) {
  if(strlen(pattern) > strlen(input)){
    return 0;
  }
  for(int i=0; i<strlen(input); i++){
    if(input[i] == pattern[0] || iFlag && tolower(input[i]) == tolower(pattern[0])){
      int breakflag = 0;
      for(int j=1; j<strlen(pattern); j++){
        if(j+i > strlen(input)){
          return 0;
        }
        if(!iFlag && pattern[j]  != input[i+j] || iFlag && tolower(pattern[j]) != tolower(input[i+j])){
          breakflag = 1;
          break;
        }
      }
      if(!breakflag){
        return 1;
      }
    }
  }
  return 0;
}

int main(int argc, char **argv){
  int iFlag = strcmp(argv[1], "-i") == 0;
  char* pattern = argv[argc - 1];
  printf("%d \n", iFlag);
  char input[100];
  while(gets(input)){
    if(hasPattern(pattern, input, iFlag)){
      puts(input);
    }
  }
  return 0;
}

