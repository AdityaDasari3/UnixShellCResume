#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "piping.h"
#include "Proclore.h"
#include "ChangeDirectory.h"
#include "Seek.h"
#include "Reveal.h"
#include "log.h"
#include "myshrc.h"
#include "GetDirectory.h"

#define MAX_COMMANDS 10
#define MAX_ARGS 20
#define PERMISSIONS 0644

// Use the same execute_command function logic from main.c to handle custom commands
void execute_single_command(char *command, char *hd, char **prev, char **prev2) {
    char *args[MAX_ARGS];
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Now handle custom commands exactly as done in main.c
    if (strcmp(args[0], "hop") == 0) {
        token = strtok(NULL, " ");
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd error");
            return;
        }

        if (token == NULL) {
            if (chdir(hd) != 0) {
                perror("chdir error");
                return;
            }
            printf("%s\n", hd);
        } else {
            while (token != NULL) {
                if (strcmp(token, "-") == 0) {
                    free(*prev2);
                    *prev2 = getcwd(NULL, 0);
                    if (*prev2 == NULL) {
                        perror("getcwd error");
                        free(hd);
                        return;
                    }
                    ChangeDirectory(hd, token, *prev);
                    strcpy(*prev, *prev2);
                } else {
                    free(*prev);
                    *prev = getcwd(NULL, 0);
                    if (*prev == NULL) {
                        perror("getcwd error");
                        free(hd);
                        return;
                    }
                    ChangeDirectory(hd, token, *prev);
                }
                token = strtok(NULL, " ");
            }
        }
    } else if (strcmp(args[0], "reveal") == 0) {
        int show_all = 0;
        int show_long = 0;
        char *path = ".";

        while ((token = strtok(NULL, " ")) != NULL) {
            if (token[0] == '-') {
                for (int j = 1; token[j] != '\0'; j++) {
                    if (token[j] == 'a') show_all = 1;
                    else if (token[j] == 'l') show_long = 1;
                    else {
                        fprintf(stderr, "\033[31mUnknown flag: %c\033[0m\n", token[j]);
                        return;
                    }
                }
            } else {
                path = token;
            }
        }
        reveal(path, show_all, show_long);
    } else if (strcmp(args[0], "seek") == 0) {
        int look_for_files = 1, look_for_dirs = 1, execute = 0;
        char *target = NULL;
        char *directory = ".";
        for (int j = 1; j < i; j++) {
            if (strcmp(args[j], "-d") == 0) look_for_files = 0;
            else if (strcmp(args[j], "-f") == 0) look_for_dirs = 0;
            else if (strcmp(args[j], "-e") == 0) execute = 1;
            else if (target == NULL) target = args[j];
            else directory = args[j];
        }
        if (target) seek(target, directory, look_for_files, look_for_dirs, execute);
    } else if (strcmp(args[0], "proclore") == 0) {
        int pid = args[1] ? atoi(args[1]) : getpid();
        proclore(pid);
    } else {
        // System command execution
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
}

int handle_io_redirection_piping(char *command, int *input_fd, int *output_fd) {
    char *input_file = NULL;
    char *output_file = NULL;
    int append_mode = 0;
    int redirection_applied = 0;

    char *output_redirect = strstr(command, ">");
    char *input_redirect = strstr(command, "<");

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
        redirection_applied = 1;
    }

    if (input_redirect) {
        *input_redirect = '\0';
        input_redirect++;
        while (*input_redirect == ' ') input_redirect++;
        input_file = input_redirect;
        char *end = strchr(input_file, ' ');
        if (end) *end = '\0';
        redirection_applied = 1;
    }

    if (input_file) {
        *input_fd = open(input_file, O_RDONLY);
        if (*input_fd < 0) {
            perror("Input file not found");
            exit(EXIT_FAILURE);
        }
    }

    if (output_file) {
        int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
        *output_fd = open(output_file, flags, PERMISSIONS);
        if (*output_fd < 0) {
            perror("Failed to open output file");
            exit(EXIT_FAILURE);
        }
    }

    return redirection_applied;
}

void handle_pipes(char *command, char *hd, char **prev, char **prev2) {
    int background = 0;
    size_t cmd_len = strlen(command);
    
    // Check if the command should run in the background
    if (cmd_len > 0 && command[cmd_len - 1] == '&') {
        background = 1;
        command[cmd_len - 1] = '\0';  // Remove the '&'
        // Trim trailing spaces
        while (cmd_len > 1 && (command[cmd_len - 2] == ' ' || command[cmd_len - 2] == '\t')) {
            command[cmd_len - 2] = '\0';
            cmd_len--;
        }
    }

    char *commands[MAX_COMMANDS];
    int i = 0;
    char *token = strtok(command, "|");
    
    while (token != NULL && i < MAX_COMMANDS - 1) {
        commands[i++] = token;
        token = strtok(NULL, "|");
    }
    commands[i] = NULL;

    int num_pipes = i - 1;
    int pipefds[2 * num_pipes];

    for (int j = 0; j < num_pipes; j++) {
        if (pipe(pipefds + j * 2) < 0) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pgid = 0;  // Process group ID for the pipeline

    for (int j = 0; j < i; j++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (j == 0) {
                // First process in the pipeline becomes the process group leader
                pgid = getpid();
                setpgid(0, pgid);
            } else {
                setpgid(0, pgid);
            }

            if (j > 0) {
                dup2(pipefds[(j - 1) * 2], STDIN_FILENO);
            }
            if (j < num_pipes) {
                dup2(pipefds[j * 2 + 1], STDOUT_FILENO);
            }

            for (int k = 0; k < 2 * num_pipes; k++) {
                close(pipefds[k]);
            }

            int input_fd = -1, output_fd = -1;
            if (j == i - 1) {  // Only apply output redirection to the last command
                handle_io_redirection_piping(commands[j], &input_fd, &output_fd);
            }
            if (input_fd != -1) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            if (output_fd != -1) {
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }

            execute_single_command(commands[j], hd, prev, prev2);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            if (j == 0) {
                pgid = pid;
            }
            setpgid(pid, pgid);
        }
    }

    // Close all pipe file descriptors in the parent
    for (int j = 0; j < 2 * num_pipes; j++) {
        close(pipefds[j]);
    }

    if (background) {
        printf("[%d] %s\n", pgid, command);
        // You might want to add the background process to your list here
        // add_bg_process(pgid, command);
    } else {
        // Wait for all processes in the pipeline to finish
        for (int j = 0; j < i; j++) {
            int status;
            waitpid(-pgid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                fprintf(stderr, "Command in pipeline failed with exit status %d\n", WEXITSTATUS(status));
            }
        }
    }
}