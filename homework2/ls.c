#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include "sort.h"

char* permissions(int bin){
  char *output = (char*)malloc(10*sizeof(char));
  output[10] = '\0';
  for(int i=0; i<9;i++){
    if((bin & 0b1) == 1){
      if(i%3==0){
        output[9-i-1] = 'x';
      }
      if(i%3==1){
        output[9-i-1] = 'w';
      }
      if(i%3==2){
        output[9-i-1] = 'r';
      }
    }else{
      output[9-i-1] ='-';
    }
    bin = bin >> 1;
  }
  return output;
}

int main(int argc, char** argv){
  char type;
  struct passwd  *pwd;
  struct group   *grp;
  struct tm *tm;
  struct dirent* dir;
  char datestring[256];
  DIR* dirp = opendir(".");
  errno = 0;
  struct stat stats;
  char input[25];
  int index = 0;
  int size = 10;
  char **values = (char**)malloc(10 * sizeof(char*));
  for(int i=0; i< 10; i++ ){ 
    values[i] = malloc(sizeof(char[25]));
  }
  while((dir = readdir(dirp)) != NULL){
    if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
      if(index >= size-1){
        size = size * 2;
        values = (char**)realloc(values, size * sizeof(char*));
        for(int i=index; i<size; i++ ){ 
          values[i] = malloc(sizeof(char[25]));
        }
      }
      strcpy(values[index], dir->d_name);
      index++;
    }
  }
  selectionSortStrings(values, index);
  if(strcmp(argv[argc-1], "-l") != 0){
    for(int i =0 ; i < index; i++){
      printf("%s\n", values[i]);
    }
    return 0;
  }
  for(int i =0 ; i < index; i++){
      stat(values[i], &stats);
      pwd = getpwuid(stats.st_uid);
      grp = getgrgid(stats.st_gid);
      tm = localtime(&stats.st_mtime);
      strftime(datestring, sizeof(datestring), "%b %d %H:%M", tm);
      char *perm = permissions(stats.st_mode & 0b111111111);
      int ft = stats.st_mode & 0b1111000000000000;
      if(ft == 16384){
        type = 'd';
      }else{
        type = '-';
      }
      printf("%c%s %s %s %d %s %s\n", type, perm,pwd->pw_name, grp->gr_name, (int)stats.st_size,datestring, values[i]);
      free(perm);
  }
  return 0;
}