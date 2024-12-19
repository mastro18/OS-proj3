#include <string.h>
#include <stdio.h>
#include <time.h>

void log_event(const char *event) {
    
    FILE *log_file = fopen("bar_log.txt", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0';

    fprintf(log_file, "[%s] %s\n", timestamp, event);
    fclose(log_file);
}
