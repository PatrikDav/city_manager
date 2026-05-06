#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "monitor_notify.h"

/*
 * monitor_notify.c - Notify monitor_reports via SIGUSR1
 */

#define PID_FILE_PATH "data/.monitor_pid"

int notify_monitor(char *out_msg, size_t out_size)
{
    int fd = open(PID_FILE_PATH, O_RDONLY);
    if (fd == -1)
    {
        snprintf(out_msg, out_size,
                 "MONITOR_NOT_INFORMED reason=pid_file_missing");
        return -1;
    }

    char buf[32];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0)
    {
        snprintf(out_msg, out_size,
                 "MONITOR_NOT_INFORMED reason=pid_file_unreadable");
        return -1;
    }
    buf[n] = '\0';

    int pid = atoi(buf);
    if (pid <= 0)
    {
        snprintf(out_msg, out_size,
                 "MONITOR_NOT_INFORMED reason=pid_file_invalid");
        return -1;
    }

    if (kill(pid, SIGUSR1) == -1)
    {
        snprintf(out_msg, out_size,
                 "MONITOR_NOT_INFORMED reason=kill_failed_%s pid=%d",
                 strerror(errno), pid);
        return -1;
    }

    snprintf(out_msg, out_size, "MONITOR_NOTIFIED pid=%d", pid);
    return 0;
}
