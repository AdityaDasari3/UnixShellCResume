#ifndef PIPING_H
#define PIPING_H

#define MAX_COMMANDS 10
#define MAX_ARGS 20


void handle_pipes(char *command, char *hd, char **prev, char **prev2);

void handle_pipes_with_redirection(char *command, char *hd, char **prev, char **prev2);

void execute_single_command(char *command, char *hd, char **prev, char **prev2);

#endif
