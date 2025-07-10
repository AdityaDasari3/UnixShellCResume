#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "Seek.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_GREEN   "\033[32m"

// Recursively search directories for matching files or directories
void seek_recursive(const char *target, const char *directory, int look_for_files, int look_for_dirs, int execute, int *found_files, int *found_dirs, char *single_match_path, const char *base_directory) {
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char path[4096];
    char relative_path[4096];

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build the full path to the entry
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        struct stat st;
        if (stat(path, &st) == -1) {
            perror("stat");
            continue;
        }

        // Compute the relative path from the base directory
        snprintf(relative_path, sizeof(relative_path), "%s/%s", directory + strlen(base_directory), entry->d_name);

        int is_file = S_ISREG(st.st_mode);
        int is_dir = S_ISDIR(st.st_mode);

        // Check if the file or directory name contains the target substring
        if ((is_file && look_for_files) || (is_dir && look_for_dirs)) {
            if (strstr(entry->d_name, target) != NULL) {
                if (is_file) {
                    (*found_files)++;
                    if (*found_files == 1 && *found_dirs == 0) {
                        strncpy(single_match_path, path, sizeof(single_match_path) - 1);
                    }
                    printf("%s%s%s\n", COLOR_GREEN, relative_path, COLOR_RESET);
                } else if (is_dir) {
                    (*found_dirs)++;
                    if (*found_dirs == 1 && *found_files == 0) {
                        strncpy(single_match_path, path, sizeof(single_match_path) - 1);
                    }
                    printf("%s%s%s\n", COLOR_BLUE, relative_path, COLOR_RESET);
                }
            }
        }

        // Recurse into subdirectories
        if (is_dir) {
            seek_recursive(target, path, look_for_files, look_for_dirs, execute, found_files, found_dirs, single_match_path, base_directory);
        }
    }

    closedir(dir);
}

// Display the contents of a file
void display_file_contents(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char ch;
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }

    fclose(file);
}

// Main seek function to search for files or directories
void seek(const char *target, const char *directory, int look_for_files, int look_for_dirs, int execute) {
    if (!look_for_files && !look_for_dirs) {
        fprintf(stderr, "Invalid flags!\n");
        return;
    }

    int found_files = 0;
    int found_dirs = 0;
    char single_match_path[4096] = "";

    // Start the recursive search from the directory
    seek_recursive(target, directory, look_for_files, look_for_dirs, execute, &found_files, &found_dirs, single_match_path, directory);

    // Handle the execute flag (if set)
    if (found_files == 0 && found_dirs == 0) {
        printf("No match found!\n");
    } else if (execute) {
        if (found_files == 1 && found_dirs == 0) {
            display_file_contents(single_match_path);
        } else if (found_dirs == 1 && found_files == 0) {
            if (access(single_match_path, X_OK) == 0) {
                if (chdir(single_match_path) == 0) {
                    printf("Changed directory to %s\n", single_match_path);
                } else {
                    perror("chdir");
                }
            } else {
                printf("Missing permissions for task!\n");
            }
        } else {
            printf("No unique match found.\n");
        }
    }
}
