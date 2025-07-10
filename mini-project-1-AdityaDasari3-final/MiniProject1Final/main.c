#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "GetDirectory.h"
#include "ChangeDirectory.h"
#include "Reveal.h"
#include "log.h"
#include "main.h"
#include "Proclore.h"
#include "Seek.h"
#include "IORedirection.h"
#include "myshrc.h"
#include "piping.h"
#include "activities.h"
#include "fgbg.h"
#include "neonate.h"
#include "iman.h"
#include "ping.h"

#define Max_Command_Length 4096
#define MAX_BG_PROCESSES 250

typedef struct {
    pid_t pid;
    char command[Max_Command_Length];
} BackgroundProcess;

BackgroundProcess bg_processes[MAX_BG_PROCESSES];
int bg_process_count = 0;
pid_t foreground_pid = -1;

void add_bg_process(pid_t pid, const char *command) {
    // Check if process already exists
    for (int i = 0; i < bg_process_count; i++) {
        if (bg_processes[i].pid == pid) {
            // Update existing process
            strncpy(bg_processes[i].command, command, Max_Command_Length - 1);
            bg_processes[i].command[Max_Command_Length - 1] = '\0';
            return;
        }
    }

    // If not found, add new process
    if (bg_process_count < MAX_BG_PROCESSES) {
        bg_processes[bg_process_count].pid = pid;
        strncpy(bg_processes[bg_process_count].command, command, Max_Command_Length - 1);
        bg_processes[bg_process_count].command[Max_Command_Length - 1] = '\0';
        bg_process_count++;
        add_process(pid, command);  // Add to activities list
        update_process_status(pid, PROCESS_RUNNING);  // Set as RUNNING by default
    }
}


void check_bg_processes() {
      for (int i = 0; i < bg_process_count; i++) {
        int status;
        pid_t result = waitpid(bg_processes[i].pid, &status, WNOHANG | WUNTRACED);
        if (result > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                printf("\nBackground process exited (%d)\n", bg_processes[i].pid);
                remove_process(result);  // Remove from activities list
                // Remove from bg_processes array
                for (int j = i; j < bg_process_count - 1; j++) {
                    bg_processes[j] = bg_processes[j + 1];
                }
                bg_process_count--;
                i--;  // Recheck this index
            } else if (WIFSTOPPED(status)) {
                update_process_status(bg_processes[i].pid, PROCESS_STOPPED);
                printf("\nBackground process stopped (%d)\n", bg_processes[i].pid);
            }
        } else if (result == 0) {
            // Process is still running, update status just in case
            update_process_status(bg_processes[i].pid, PROCESS_RUNNING);
        } else if (errno == ECHILD) {
            // Process no longer exists, remove it
            remove_process(bg_processes[i].pid);
            for (int j = i; j < bg_process_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_process_count--;
            i--;  // Recheck this index
        } else {
            // Other error occurred
            perror("waitpid");
        }
    }
}

void sigint_handler(int sig) {
    if (foreground_pid != -1) {
        kill(foreground_pid, SIGINT);
        printf("\nSent SIGINT to process %d\n", foreground_pid);
        foreground_pid = -1;
    }
}


void sigtstp_handler(int sig) {
    if (foreground_pid != -1) {
        
        kill(foreground_pid, SIGTSTP);
        printf("\nStopped process %d\n", foreground_pid);
        
        update_process_status(foreground_pid, PROCESS_STOPPED);  
        foreground_pid = -1;
    }
}

void execute_command(char *command, char *hd, char **prev, char **prev2) {
    
    char *token = strtok(command, " ");
    if (token == NULL) {
        return;
    }

    if (strcmp(token, "hop") == 0) {
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
    } 
    else if(strcmp(token, "activities") == 0){
        
        print_activities();
    }
    else if (strcmp(token, "iMan") == 0) {
         
    token = strtok(NULL, " ");
    if (token != NULL) {
        
        fetch_man_page(token);
    } else {
        fprintf(stderr, "iMan: missing command name\n");
    }
    }
    else if(strcmp(token, "fg") == 0) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            fprintf(stderr, "fg: missing pid argument\n");
        } else {
            pid_t pid = atoi(token);
            fg_command(pid);
        }
    } 
    else if (strcmp(token, "bg") == 0) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            fprintf(stderr, "bg: missing pid argument\n");
        } else {
            pid_t pid = atoi(token);
            bg_command(pid);
        }
    } 
    else if (strcmp(token, "reveal") == 0) {
        int show_all = 0;
        int show_long = 0;
        char *path = ".";

        while ((token = strtok(NULL, " ")) != NULL) {
            if (token[0] == '-') {
                for (int j = 1; token[j] != '\0'; j++) {
                    if (token[j] == 'a') {
                        show_all = 1;
                    } else if (token[j] == 'l') {
                        show_long = 1;
                    } else {
                        fprintf(stderr, "\033[31mUnknown flag: %c\033[0m\n", token[j]);
                        return;
                    }
                }
            } else {
                path = token;
            }
        }

        reveal(path, show_all, show_long);
    } else if (strcmp(token, "log") == 0) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            print_log();
        } else if (strcmp(token, "purge") == 0) {
            purge_log();
        } else if (strcmp(token, "execute") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                int index = atoi(token);
                execute_log_command(index, hd, prev, prev2);
            } else {
                fprintf(stderr, "log execute requires an index\n");
            }
        } else {
            fprintf(stderr, "Unknown log command: %s\n", token);
        }
    } 
    else if (strcmp(token, "neonate") == 0) {
    token = strtok(NULL, " ");
    if (token != NULL && strcmp(token, "-n") == 0) {
        token = strtok(NULL, " ");
        if (token != NULL) {
            int time_arg = atoi(token);
            if (time_arg > 0) {
                neonate_command(time_arg);
            } else {
                fprintf(stderr, "Invalid time argument for neonate\n");
            }
        } else {
            fprintf(stderr, "Missing time argument for neonate\n");
        }
    } else {
        fprintf(stderr, "Invalid syntax for neonate command\n");
    }
    }
    else if (strcmp(token, "ping") == 0) {
    token = strtok(NULL, " ");
    if (token == NULL) {
        fprintf(stderr, "ping: missing pid\n");
        return;
    }
    pid_t pid = atoi(token);
    
    token = strtok(NULL, " ");
    if (token == NULL) {
        fprintf(stderr, "ping: missing signal number\n");
        return;
    }
    int signal_number = atoi(token);
    
    ping_process(pid, signal_number);
    }
    else if (strcmp(token, "seek") == 0) {
        int look_for_files = 1, look_for_dirs = 1, execute = 0;
        char *target = NULL;
        char *directory = "."; // Default to current directory

        while ((token = strtok(NULL, " ")) != NULL) {
            if (token[0] == '-') {
                if (strcmp(token, "-d") == 0) {
                    look_for_files = 0;
                } else if (strcmp(token, "-f") == 0) {
                    look_for_dirs = 0;
                } else if (strcmp(token, "-e") == 0) {
                    execute = 1;
                } else {
                    fprintf(stderr, "Unknown flag: %s\n", token);
                    return;
                }
            } else if (target == NULL) {
                target = token;
            } else {
                directory = token; // Assume it's the directory
            }
        }

        if (look_for_files == 0 && look_for_dirs == 0) {
            fprintf(stderr, "Invalid flags!\n");
            return;
        }

        if (target == NULL) {
            fprintf(stderr, "seek: missing target name\n");
            return;
        }

        seek(target, directory, look_for_files, look_for_dirs, execute);
    } else {
        // System command execution
        char *args[64];
        int i = 0;
        char *arg = strtok(command, " ");
        while (arg != NULL && i < 63) {
            args[i++] = arg;
            arg = strtok(NULL, " ");
        }
        args[i] = NULL;

        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args); // Execute system command
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0); // Wait for the command to finish
        }
    }
}

int main() {
    load_myshrc();  // Load the aliases and functions from .myshrc
    const char *username = getenv("USER");
    if (username == NULL) {
        fprintf(stderr, "\033[31mError: USER environment variable not set.\033[0m\n");
        exit(EXIT_FAILURE);
    }

    char hostname[1024];
    if (gethostname(hostname, sizeof(hostname)) < 0) {
        perror("gethostname error");
        exit(EXIT_FAILURE);
    }

    char *hd = getcwd(NULL, 0);
    if (hd == NULL) {
        perror("getcwd error");
        exit(EXIT_FAILURE);
    }
    char *prev = getcwd(NULL, 0);
    if (prev == NULL) {
        perror("getcwd error");
        exit(EXIT_FAILURE);
    }
    char *prev2 = getcwd(NULL, 0);
    if (prev2 == NULL) {
        perror("getcwd error");
        exit(EXIT_FAILURE);
    }

    init_log(); // Initialize the log by loading from file

    time_t last_command_time = 0;
    char last_command[Max_Command_Length] = "";
    int last_command_duration = 0;
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    while (1) {
        check_bg_processes();

        if (last_command_duration > 2) {
            printf("<%s@%s:%s took %ds>\n", username, hostname, last_command, last_command_duration);
            last_command_duration = 0; // Reset after displaying
        } else {
            printf("<%s@%s:", username, hostname);
            display_directory(hd);
        }

        char command_line[Max_Command_Length];
        if (fgets(command_line, sizeof(command_line), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\nLogging out...\n");
                // Kill all background processes
                for (int i = 0; i < bg_process_count; i++) {
                    kill(bg_processes[i].pid, SIGKILL);
                }
                break;  // Exit the shell
            }
            else{perror("fgets error");
            free(hd);
            free(prev);
            free(prev2);
            exit(EXIT_FAILURE);
            }
        }

        command_line[strcspn(command_line, "\n")] = '\0'; // Remove newline

        char *saveptr1;
        char *command = strtok_r(command_line, ";", &saveptr1);
        while (command != NULL) {
            // Trim leading and trailing spaces/tabs
            while (*command == ' ' || *command == '\t') command++;
            char *end = command + strlen(command) - 1;
            while (end > command && (*end == ' ' || *end == '\t')) end--;
            *(end + 1) = '\0';

            if (*command != '\0') {
                // Check if the command should run in the background
                int background = 0;
                char *bg_char = strrchr(command, '&');
                if (bg_char != NULL && (bg_char == command || *(bg_char - 1) == ' ')) {
                    background = 1;
                    *bg_char = '\0';  // Remove the '&' from the command
                    // Trim trailing spaces again
                    end = bg_char - 1;
                    while (end > command && (*end == ' ' || *end == '\t')) end--;
                    *(end + 1) = '\0';
                }

                // Store command in history (except for 'log' commands)
                if (strstr(command, "log") == NULL) {
                    store_command(command);
                }

                // Resolve alias
                char *resolved_command = resolve_alias(command);
                if (resolved_command != NULL) {
                    strcpy(command, resolved_command);
                }

                // Check for piping or redirection
                if (strchr(command, '|')) {
                    handle_pipes(command, hd, &prev, &prev2);
                } else if (strchr(command, '>') || strchr(command, '<') || strstr(command, ">>")) {
                    handle_io_redirection(command, hd, &prev, &prev2);
                } else {
                    
                    char *first_word = strtok(strdup(command), " \t");
                    int is_custom_command = (strcmp(first_word, "hop") == 0 ||
                                             strcmp(first_word, "reveal") == 0 ||
                                             strcmp(first_word, "log") == 0 ||
                                             strcmp(first_word, "proclore") == 0 ||
                                             strcmp(first_word, "seek") == 0 ||
                                             strcmp(first_word, "mk_hop") == 0 ||
                                             strcmp(first_word, "hop_seek") == 0 ||
                                             strcmp(first_word, "activities") == 0 ||
                                             strcmp(first_word, "fg") == 0 ||
                                            strcmp(first_word, "bg") == 0 ||
                                            strcmp(first_word, "neonate") == 0 ||
                                            strcmp(first_word, "iMan") == 0 ||
                                            strcmp(first_word, "ping") == 0);
                    free(first_word);

                    if (is_custom_command) {
                        
                        time_t start_time = time(NULL);
                        execute_command(command, hd, &prev, &prev2);
                        time_t end_time = time(NULL);
                        last_command_duration = (int)(end_time - start_time);
                        strncpy(last_command, command, Max_Command_Length - 1);
                        last_command[Max_Command_Length - 1] = '\0';
                    } else {
                        time_t start_time = time(NULL);
                        pid_t pid = fork();
                        if (pid == 0) {
                            
                            setpgid(0, 0);  
                            signal(SIGINT, SIG_DFL);   
                            signal(SIGTSTP, SIG_DFL);

                            char *args[64];
                            int i = 0;
                            char *arg = strtok(command, " \t");
                            while (arg != NULL && i < 63) {
                                args[i++] = arg;
                                arg = strtok(NULL, " \t");
                            }
                            args[i] = NULL;

                            execvp(args[0], args);
                            perror("execvp failed");
                            exit(EXIT_FAILURE);
                        } else if (pid > 0) {
                            setpgid(pid, pid);
                            if (!background) {
                            foreground_pid = pid;
                            int status;
                            waitpid(pid, &status, WUNTRACED);
                            foreground_pid = -1;
                            if (WIFSTOPPED(status)) {
                                printf("Process %d stopped\n", pid);
                                add_bg_process(pid, command);
                                update_process_status(pid, PROCESS_STOPPED);
                            }
                            time_t end_time = time(NULL);
                            last_command_duration = (int)(end_time - start_time);
                            strncpy(last_command, command, Max_Command_Length - 1);
                            last_command[Max_Command_Length - 1] = '\0';
                        } else {
                            printf("[%d] %d\n", bg_process_count + 1, pid);
                            add_bg_process(pid, command);
                        }
                    } else {
                            perror("fork failed");
                        }
                    }
                }
            }

            command = strtok_r(NULL, ";", &saveptr1);
        }
    }

    free(hd);
    free(prev);
    free(prev2);
    return 0;
}
