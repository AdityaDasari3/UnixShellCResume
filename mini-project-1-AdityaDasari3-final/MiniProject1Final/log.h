#ifndef LOG_H
#define LOG_H

#define MAX_LOG_SIZE 15
#define LOG_FILE_PATH "command_log.txt"

extern void init_log();
extern void store_command(const char *command);

extern void print_log();
extern void purge_log();
extern void execute_log_command(int index, char *hd, char **prev, char **prev2);

#endif
