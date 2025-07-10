#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "Reveal.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_WHITE   "\033[37m"

void display_file_info(const char *path) {
    struct stat file_stat;
    
    if (stat(path, &file_stat) == -1) {
        perror("stat error");
        return;
    }

    if (S_ISREG(file_stat.st_mode)) {
        // Print detailed info only for regular files
        printf((S_ISREG(file_stat.st_mode)) ? "-" : "?");
        printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
        printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
        printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
        printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
        printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
        printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
        printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
        printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
        printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");

        printf(" %lu", file_stat.st_nlink);

        struct passwd *pwd = getpwuid(file_stat.st_uid);
        struct group *grp = getgrgid(file_stat.st_gid);
        printf(" %s %s", pwd->pw_name, grp->gr_name);

        printf(" %5ld", file_stat.st_size);

        char timebuf[80];
        struct tm *timeinfo;
        timeinfo = localtime(&file_stat.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
        printf(" %s", timebuf);

        printf(" %s\n", path);
    } else if (S_ISDIR(file_stat.st_mode)) {
        printf("This is a directory, not a file.\n");
    } else {
        printf("This is not a regular file.\n");
    }
}

int compare_entries(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

void reveal_directory(const char *path, int show_all) {
    struct dirent **namelist;
    int n;

    n = scandir(path, &namelist, NULL, compare_entries);
    if (n < 0) {
        perror("scandir error");
        return;
    }

    for (int i = 0; i < n; i++) {
        struct dirent *entry = namelist[i];

        if (!show_all && entry->d_name[0] == '.') {
            free(namelist[i]);
            continue;
        }

        struct stat file_stat;
        char full_path[4096];

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &file_stat) == -1) {
            perror("stat error");
            free(namelist[i]);
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            printf("%s%s%s\n", COLOR_BLUE, entry->d_name, COLOR_RESET);
        } else if (file_stat.st_mode & S_IXUSR) {
            printf("%s%s%s\n", COLOR_GREEN, entry->d_name, COLOR_RESET);
        } else {
            printf("%s%s%s\n", COLOR_WHITE, entry->d_name, COLOR_RESET);
        }

        free(namelist[i]);
    }
    free(namelist);
}

void reveal(const char *path, int show_all, int show_long) {
    struct stat path_stat;
    if (stat(path, &path_stat) == -1) {
        perror("stat error");
        return;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        reveal_directory(path, show_all);
    } else if (S_ISREG(path_stat.st_mode) && show_long) {
        display_file_info(path);
    } else {
        printf("%s%s%s\n", COLOR_WHITE, path, COLOR_RESET);
    }
}
