#ifndef MYSHRC_H
#define MYSHRC_H

#define MAX_ALIASES 100
#define MAX_ALIAS_LENGTH 128
#define MAX_COMMAND_LENGTH 400

typedef struct {
    char alias[MAX_ALIAS_LENGTH];
    char command[MAX_COMMAND_LENGTH];
} Alias;

void load_myshrc();

int check_alias(char *command);
char* resolve_alias(char* command);
void mk_hop(char *directory);
void hop_seek(char *directory);

#endif
