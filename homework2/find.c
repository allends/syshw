#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void find(char *path, char *string) {
  DIR *dir;
  struct dirent *entry;
  dir = opendir(path);
  
  if (dir == NULL) {
    return;

  }

  while ((entry = readdir(dir)) != NULL) {
    if(entry->d_type == DT_DIR) {
      
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
         
        find(entry->d_name, string);
        printf("%s/",entry->d_name);
      }
      else
      {
       //do nothing
      }
      } else {
        
        if(strstr(entry->d_name,string))
        {
          printf("%s\n", entry->d_name);
        }
      }
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {


  }
   else
   {
    
      find(".",argv[1]);
   }
   return 0;
}