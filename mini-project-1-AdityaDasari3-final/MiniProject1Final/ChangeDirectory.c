#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "ChangeDirectory.h"

int is_directory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) == 0) {
        return S_ISDIR(path_stat.st_mode);
    } else {
        perror("stat error");
        return 0;
    }
}

void ChangeDirectory(char *hd, char *command,char *prev) {
    
    if (strcmp(command, "~") == 0) {
        if (chdir(hd) != 0) {
            perror("chdir error");
            return;
        }
        printf("%s\n", hd);
        return;
    }

    if (command[0] == '~' && command[1] == '/') {
        char path[1024];
        snprintf(path, sizeof(path), "%s%s", hd, command + 1); 

        if (chdir(path) != 0) {
            perror("chdir error");
            return;
        }
        printf("%s\n", path);
        return;
    }

    if (strcmp(command, "..") == 0) {
        if (chdir("..") != 0) {
            perror("chdir error");
            return;
        }
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd error");
        }
        return;
    }

    if (strcmp(command, ".") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd error");
        }
        return;
    }

    if (strcmp(command, "-") == 0) {
        
        if (chdir(prev) != 0) {
            perror("chdir error");
            return;
        }
        printf("%s\n", prev);
        return;
    }

    if (is_directory(command)) {
        if (chdir(command) != 0) {
            perror("chdir error");
            return;
        }
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd error");
        }
    } else {
        fprintf(stderr, "\033[31mError: '%s' is not a directory or does not exist.\033[0m\n", command); // Red color for error
    }
}
