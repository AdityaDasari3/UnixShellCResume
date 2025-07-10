#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "Proclore.h"

void proclore(int pid) {
    char path[256];
    char buffer[1024];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *stat_file = fopen(path, "r");
    if (stat_file == NULL) {
        if (errno == EACCES) {
            fprintf(stderr, "error: permission to view process info is not granted.\n");
        } else {
            perror("fopen");
        }
        return;
    }

    if (fgets(buffer, sizeof(buffer), stat_file) == NULL) {
        perror("fgets");
        fclose(stat_file);
        return;
    }

    fclose(stat_file);

    // Parse the information needed
    int ppid, pgrp;
    char state;
    unsigned long vsize;
    char comm[256];

    sscanf(buffer, "%d %s %c %d %d %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %lu",
           &pid, comm, &state, &ppid, &pgrp, &vsize);

    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, buffer, sizeof(buffer) - 1);
    if (len == -1) {
        perror("readlink");
        return;
    }
    buffer[len] = '\0';

    printf("PID: %d\n", pid);
    printf("Process Status: %c\n", state);
    printf("Process Group: %d\n", pgrp);
    printf("Virtual Memory: %lu\n", vsize);
    printf("Executable Path: %s\n", buffer);
}
