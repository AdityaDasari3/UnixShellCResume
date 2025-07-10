#include "neonate.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

pid_t get_most_recent_pid() {
    DIR *dir;
    struct dirent *entry;
    pid_t max_pid = 0;
    time_t max_time = 0;

    dir = opendir("/proc");
    if (dir == NULL) {
        perror("Unable to open /proc");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Check if the directory name is a number (PID)
        int is_pid = 1;
        for (int i = 0; entry->d_name[i] != '\0'; i++) {
            if (!isdigit(entry->d_name[i])) {
                is_pid = 0;
                break;
            }
        }

        if (is_pid) {
            pid_t pid = atoi(entry->d_name);
            if (pid > 0) {
                char path[256];
                struct stat st;
                snprintf(path, sizeof(path), "/proc/%d", pid);
                if (stat(path, &st) == 0) {
                    if (st.st_ctime > max_time) {
                        max_time = st.st_ctime;
                        max_pid = pid;
                    }
                }
            }
        }
    }

    closedir(dir);
    return max_pid;
}
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void neonate_command(int time_arg) {
    while (1) {
        pid_t recent_pid = get_most_recent_pid();
        if (recent_pid > 0) {
            printf("%d\n", recent_pid);
        } else {
            printf("Unable to get most recent PID\n");
        }

        for (int i = 0; i < time_arg; i++) {
            sleep(1);
            if (kbhit()) {
                char c = getchar();
                if (c == 'x' || c == 'X') {
                    return;
                }
            }
        }
    }
}