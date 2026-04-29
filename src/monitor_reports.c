#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

/*
 * monitor_reports.c - Background monitor for new reports
 */

#define PID_FILE_PATH "data/.monitor_pid"

static int write_pid_file(void)
{
    if (mkdir("data", 0750) == -1 && errno != EEXIST)
    {
        perror("monitor_reports: cannot create data/");
        return -1;
    }

    int fd = open(PID_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("monitor_reports: open .monitor_pid failed");
        return -1;
    }

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    if (write(fd, buf, (size_t)len) != (ssize_t)len)
    {
        perror("monitor_reports: write .monitor_pid failed");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int main(void)
{
    if (write_pid_file() != 0)
        return EXIT_FAILURE;

    const char *msg = "monitor_reports: started, waiting for signals\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    for (;;)
        pause();

    return EXIT_SUCCESS;
}
