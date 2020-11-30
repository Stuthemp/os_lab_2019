#include "revert_string.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void RevertString(char *str)
{
	char temp;
    int c;

    scanf("%d", &c);

    while (c > strlen(str)/2){
        scanf("%d", &c);}

	for(int i = 0; i < c; i++) {
    temp=str[i];
    str[i]=str[strlen(str)-i-1];
    str[strlen(str)-i-1]=temp;
    
    }
}

