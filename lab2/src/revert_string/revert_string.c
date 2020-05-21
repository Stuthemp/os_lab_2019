#include "revert_string.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void RevertString(char *str)
{
	char  *revert_string;
    revert_string = (char*)malloc(sizeof(char) * (strlen(str) + 1));
    for(int i =0; i < strlen(str); i++){
        revert_string[i] = str[strlen(str)-i-1];
    }
    strcpy(str,revert_string);
    free(revert_string);
}

