#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "IORedirection.h"
#include "ChangeDirectory.h"
#include "Seek.h"
#include "Reveal.h"
#include "log.h"
#include "Proclore.h"
#include "myshrc.h"
#include "GetDirectory.h"

#define PERMISSIONS 0644
#define MAX_ARGS 64

void execute_command_IO(char *command, char *hd, char **prev, char **prev2) {
    char *args[MAX_ARGS];
    int arg_count = 0;
    char *token = strtok(command, " ");

    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    if (arg_count == 0) return;

    if (strcmp(args[0], "hop") == 0) {
        if (arg_count == 1) {
            if (chdir(hd) != 0) perror("chdir error");
            else printf("%s\n", hd);
        } else {
            for (int i = 1; i < arg_count; i++) {
                if (strcmp(args[i], "-") == 0) {
                    char *temp = *prev2;
                    *prev2 = getcwd(NULL, 0);
                    ChangeDirectory(hd, args[i], *prev);
                    free(*prev);
                    *prev = temp;
                } else {
                    free(*prev);
                    *prev = getcwd(NULL, 0);
                    ChangeDirectory(hd, args[i], *prev);
                }
            }
        }
    } else if (strcmp(args[0], "reveal") == 0) {
        int show_all = 0, show_long = 0;
        char *path = ".";
        for (int i = 1; i < arg_count; i++) {
            if (args[i][0] == '-') {
                for (int j = 1; args[i][j] != '\0'; j++) {
                    if (args[i][j] == 'a') show_all = 1;
                    else if (args[i][j] == 'l') show_long = 1;
                    else fprintf(stderr, "Unknown flag: %c\n", args[i][j]);
                }
            } else {
                path = args[i];
            }
        }
        reveal(path, show_all, show_long);
    } else if (strcmp(args[0], "log") == 0) {
        if (arg_count == 1) {
            print_log();
        } else if (strcmp(args[1], "purge") == 0) {
            purge_log();
        } else if (strcmp(args[1], "execute") == 0 && arg_count > 2) {
            int index = atoi(args[2]);
            execute_log_command(index, hd, prev, prev2);
        } else {
            fprintf(stderr, "Unknown log command\n");
        }
    } else if (strcmp(args[0], "seek") == 0) {
        int look_for_files = 1, look_for_dirs = 1, execute = 0;
        char *target = NULL, *directory = ".";
        for (int i = 1; i < arg_count; i++) {
            if (strcmp(args[i], "-d") == 0) look_for_files = 0;
            else if (strcmp(args[i], "-f") == 0) look_for_dirs = 0;
            else if (strcmp(args[i], "-e") == 0) execute = 1;
            else if (target == NULL) target = args[i];
            else directory = args[i];
        }
        if (target) seek(target, directory, look_for_files, look_for_dirs, execute);
    } else if (strcmp(args[0], "proclore") == 0) {
        int pid = (arg_count > 1) ? atoi(args[1]) : getpid();
        proclore(pid);
    } else {
        // System command execution
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
}

void handle_io_redirection(char *command, char *hd, char **prev, char **prev2) {
    char *input_file = NULL;
    char *output_file = NULL;
    int append_mode = 0;

    // Locate redirection symbols
    char *output_redirect = strstr(command, ">");
    char *input_redirect = strstr(command, "<");

    // Handle output redirection
    if (output_redirect) {
        *output_redirect = '\0';
        output_redirect++;
        if (*output_redirect == '>') {
            append_mode = 1;
            output_redirect++;
        }
        while (*output_redirect == ' ') output_redirect++;
        output_file = output_redirect;
        char *end = strchr(output_file, ' ');
        if (end) *end = '\0';
    }

    // Handle input redirection
    if (input_redirect) {
        *input_redirect = '\0';
        input_redirect++;
        while (*input_redirect == ' ') input_redirect++;
        input_file = input_redirect;
        char *end = strchr(input_file, ' ');
        if (end) *end = '\0';
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (input_file) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("No such input file found");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (output_file) {
            int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
            int fd = open(output_file, flags, PERMISSIONS);
            if (fd < 0) {
                perror("Cannot open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execute_command_IO(command, hd, prev, prev2);
        exit(EXIT_SUCCESS);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command failed with exit status %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork failed");
    }
}