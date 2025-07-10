#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "main.h" 

#define Max_Command_Length 4096

static char log_commands[MAX_LOG_SIZE][Max_Command_Length];
static int log_count = 0;

void init_log() {
    FILE *file = fopen(LOG_FILE_PATH, "r");
    if (file != NULL) {
        log_count = 0;
        while (fgets(log_commands[log_count], Max_Command_Length, file) != NULL && log_count < MAX_LOG_SIZE) {
            log_commands[log_count][strcspn(log_commands[log_count], "\n")] = '\0';
            log_count++;
        }
        fclose(file);
    }
}

void store_command(const char *command) {
    if (strstr(command, "log") != NULL) {
        return; 
    }
    if (log_count > 0 && strcmp(log_commands[(log_count - 1) % MAX_LOG_SIZE], command) == 0) {
        return; 
    }

    if (log_count < MAX_LOG_SIZE) {
        strcpy(log_commands[log_count], command);
        log_count++;
    } else {
        for (int i = 0; i < MAX_LOG_SIZE - 1; i++) {
            strcpy(log_commands[i], log_commands[i + 1]);
        }
        strcpy(log_commands[MAX_LOG_SIZE - 1], command);
    }

    FILE *file = fopen(LOG_FILE_PATH, "w");
    if (file == NULL) {
        perror("Error opening log file");
        return;
    }

    for (int i = 0; i < log_count; i++) {
        fprintf(file, "%s\n", log_commands[i]);
    }

    fclose(file);
}

void print_log() {
    for (int i = 0; i < log_count; i++) {
        printf("%s\n",  log_commands[i]); 
    }
}

void purge_log() {
    log_count = 0;
    FILE *file = fopen(LOG_FILE_PATH, "w");
    if (file != NULL) {
        fclose(file);
    }
}

void execute_log_command(int index, char *hd, char **prev, char **prev2) {
    if (index < 1 || index > log_count) {
        printf("Invalid index\n");
        return;
    }

  
    int actual_index = log_count - index;

    char command[Max_Command_Length];
    strcpy(command, log_commands[actual_index]);

    printf("Executing: %s\n", command);

    
    execute_command(command, hd, prev, prev2);
}
