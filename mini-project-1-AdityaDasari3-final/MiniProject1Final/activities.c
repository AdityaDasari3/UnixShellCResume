#include "activities.h"
#include <stdio.h>
#include <string.h>

// Define the global variables (not static)
Process process_list[MAX_PROCESSES];
int process_count = 0;

void add_process(pid_t pid, const char *command) {
    if (process_count < MAX_PROCESSES) {
        process_list[process_count].pid = pid;
        strncpy(process_list[process_count].command, command, sizeof(process_list[process_count].command) - 1);
        process_list[process_count].command[sizeof(process_list[process_count].command) - 1] = '\0';
        process_list[process_count].status = PROCESS_RUNNING;
        process_count++;
    }
}

void remove_process(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            for (int j = i; j < process_count - 1; j++) {
                process_list[j] = process_list[j + 1];
            }
            process_count--;
            break;
        }
    }
}

void update_process_status(pid_t pid, ProcessStatus status) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            process_list[i].status = status;
            break;
        }
    }
}

void print_activities() {
    for (int i = 0; i < process_count; i++) {
        printf("%d : %s - %s\n", 
               process_list[i].pid, 
               process_list[i].command, 
               process_list[i].status == PROCESS_RUNNING ? "Running" : "Stopped");
    }
}