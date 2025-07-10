#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "GetDirectory.h"

void display_directory(char *hd){

    char *cwd=getcwd(NULL, 0);
    if (cwd != NULL) {

    if (strncmp(cwd, hd, strlen(hd)) == 0) {
            if(strcmp(cwd,hd)==0){
                printf("~> ");
            }
            else{
            
            printf("%s> ", cwd + strlen(hd));
        } 
        }
        else {
            
            printf("%s> ", cwd);
           
        }
    free(cwd); 
} else {
    perror("getcwd error");
}
    
}

