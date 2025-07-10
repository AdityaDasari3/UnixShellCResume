#ifndef REVEAL_H
#define REVEAL_H
#include <dirent.h>
extern void display_file_info(const char *path);
extern void reveal_directory(const char *path, int show_all);
extern int compare_entries(const struct dirent **a, const struct dirent **b);
extern void reveal(const char *path, int show_all, int show_long);

#endif