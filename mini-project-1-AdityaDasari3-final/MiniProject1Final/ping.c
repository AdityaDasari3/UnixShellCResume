#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "ping.h"
#include "activities.h"  // Include this to access update_process_status

int ping_process(pid_t pid, int signal_number) {
    // Take modulo 32 of the signal number
   signal_number %= 32;

    // Send the signal
    if (kill(pid, signal_number) == -1) {
        if (errno == ESRCH) {
            fprintf(stderr, "No such process found\n");
        } else {
            fprintf(stderr, "Error sending signal: %s\n", strerror(errno));
        }
        return -1;
    }

    printf("Sent signal %d to process with pid %d\n", signal_number, pid);

    // Update process status based on the signal
    if (signal_number == SIGKILL || signal_number == SIGTERM) {
        remove_process(pid);  
    } else if (signal_number == SIGSTOP || signal_number == SIGTSTP) {
        update_process_status(pid, PROCESS_STOPPED);
    } else if (signal_number == SIGCONT) {
        update_process_status(pid, PROCESS_RUNNING);
    }

    return 0;
}