#include "fgbg.h"
#include "activities.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

extern Process process_list[MAX_PROCESSES];
extern int process_count;

void fg_command(pid_t pid) {
    int found = 0;
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            found = 1;
            if (kill(pid, SIGCONT) < 0) {
                perror("kill (SIGCONT)");
                return;
            }
            
            printf("Bringing process %d (%s) to foreground\n", pid, process_list[i].command);
            
            int status;
            if (waitpid(pid, &status, WUNTRACED) == -1) {
                perror("waitpid");
                return;
            }
            
            if (WIFSTOPPED(status)) {
                update_process_status(pid, PROCESS_STOPPED);
                printf("Process %d stopped\n", pid);
            } else {
                // Process terminated
                remove_process(pid);
            }
            break;
        }
    }
    
    if (!found) {
        printf("No such process found\n");
    }
}

void bg_command(pid_t pid) {
    int found = 0;
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            found = 1;
            if (process_list[i].status == PROCESS_RUNNING) {
                printf("Process %d is running\n", pid);
            } else {
                if (kill(pid, SIGCONT) < 0) {
                    perror("kill (SIGCONT)");
                    return;
                }
                update_process_status(pid, PROCESS_RUNNING);
                printf("Changed state of process %d (%s) to running\n", pid, process_list[i].command);
            }
            break;
        }
    }
    
    if (!found) {
        printf("No such process found\n");
    }
}