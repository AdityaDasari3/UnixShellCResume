#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <sys/types.h>

#define MAX_PROCESSES 100

typedef enum {
    PROCESS_RUNNING,
    PROCESS_STOPPED
} ProcessStatus;

typedef struct {
    pid_t pid;
    char command[256];
    ProcessStatus status;
} Process;

extern Process process_list[MAX_PROCESSES];
extern int process_count;

void add_process(pid_t pid, const char *command);
void remove_process(pid_t pid);
void update_process_status(pid_t pid, ProcessStatus status);
void print_activities();

#endif