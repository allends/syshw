#include <stdlib.h>
#include <string.h>

void selectionSortStrings(char* arr[], int n)
{
    int i, j;
    int min_idx;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (strcasecmp(arr[j], arr[min_idx]) < 0)
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