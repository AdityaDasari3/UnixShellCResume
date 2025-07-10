#ifndef PING_H
#define PING_H

#include <sys/types.h>

// Function to send a signal to a process
int ping_process(pid_t pid, int signal_number);

#endif