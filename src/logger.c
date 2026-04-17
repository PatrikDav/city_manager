#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "logger.h"

/*
 * logger.c - Operation Logging to logged_district
 */

// log_operation
int log_operation(const char *district, const char *role_str, const char *username, const char *operation)
{
    char path[256];
    char timestamp[32];
    char log_line[512];

    snprintf(path, sizeof(path), "data/%s/logged_district", district);

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    snprintf(log_line, sizeof(log_line),
             "[%s] role=%s user=%s op=%s\n",
             timestamp, role_str, username, operation);

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1)
    {
        perror("log_operation: open failed");
        return -1;
    }

    ssize_t len = (ssize_t)strlen(log_line);
    if (write(fd, log_line, (size_t)len) != len)
    {
        perror("log_operation: write failed");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
