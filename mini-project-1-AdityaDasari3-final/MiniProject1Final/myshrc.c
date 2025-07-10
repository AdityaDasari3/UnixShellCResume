#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "myshrc.h"
#include "ChangeDirectory.h"
#include "Seek.h"

#define MAX_ALIASES 100
#define MAX_ALIAS_LENGTH 128

Alias alias_table[MAX_ALIASES];
int alias_count = 0;

void load_myshrc() {
    FILE *file = fopen(".myshrc", "r");
    if (!file) {
        perror("Failed to load .myshrc");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "alias", 5) == 0) {
            char *alias_name = strtok(line + 6, " =\n");  // Trim 'alias ' and then split on ' ' or '='
            char *alias_command = strtok(NULL, "=");

            if (alias_name && alias_command) {
                // Trim any leading or trailing whitespace from alias_command
                while (*alias_command == ' ') alias_command++;  // Trim leading spaces
                char *end = alias_command + strlen(alias_command) - 1;
                while (end > alias_command && (*end == ' ' || *end == '\n')) *end-- = '\0';  // Trim trailing spaces

                strncpy(alias_table[alias_count].alias, alias_name, MAX_ALIAS_LENGTH - 1);
                strncpy(alias_table[alias_count].command, alias_command, MAX_COMMAND_LENGTH - 1);
                alias_table[alias_count].alias[MAX_ALIAS_LENGTH - 1] = '\0';
                alias_table[alias_count].command[MAX_COMMAND_LENGTH - 1] = '\0';
                alias_count++;
            }
        }
    }
    fclose(file);
}

char* resolve_alias(char* command) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(command, alias_table[i].alias) == 0) {
            return alias_table[i].command;
        }
    }
    return NULL;  // Return NULL if no alias is found
}

void mk_hop(char *directory) {
    if (mkdir(directory, 0755) == -1) {
        perror("mkdir failed");
    }
    if (chdir(directory) != 0) {
        perror("chdir failed");
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd); 
        }
    }
}

void hop_seek(char *directory) {
    if (chdir(directory) != 0) {
        perror("chdir failed");
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd); 
        }
    }

    seek(directory, ".", 1, 1, 0); 
}
