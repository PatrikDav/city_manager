#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

/*
 * monitor_reports.c - Background monitor for new reports
 *
 * All output uses a structured line format so hub_mon can parse it:
 *   INFO:<message>\n
 *   ERROR:<message>\n
 *   NOTIFY:<message>\n
 *
 * When launched by hub_mon, stdout is already redirected to a pipe via
 * dup2() before exec, so all write()s to STDOUT_FILENO reach hub_mon.
 */

#define PID_FILE_PATH "data/.monitor_pid"

static volatile sig_atomic_t stop_flag = 0;

static void on_sigint(int signo)
{
    (void)signo;
    static const char msg[] = "INFO:monitor shutting down\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    stop_flag = 1;
}

static void on_sigusr1(int signo)
{
    (void)signo;
    static const char msg[] = "NOTIFY:new report added\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

static int install_handlers(void)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGUSR1);
    sa.sa_flags = 0;

    sa.sa_handler = on_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        return -1;

    sa.sa_handler = on_sigusr1;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        return -1;

    return 0;
}

/* Returns the PID from the existing PID file if that process is still alive,
 * or 0 if the file is absent / stale. */
static pid_t check_existing_monitor(void)
{
    int fd = open(PID_FILE_PATH, O_RDONLY);
    if (fd == -1)
        return 0;

    char buf[32] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0)
        return 0;

    pid_t pid = (pid_t)atoi(buf);
    if (pid <= 0)
        return 0;

    /* kill(pid, 0) succeeds if the process exists */
    if (kill(pid, 0) == 0)
        return pid;

    return 0;
}

static int write_pid_file(void)
{
    if (mkdir("data", 0750) == -1 && errno != EEXIST)
        return -1;

    int fd = open(PID_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        return -1;

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    if (write(fd, buf, (size_t)len) != (ssize_t)len)
    {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static void write_msg(const char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main(void)
{
    /* Check for a running monitor before doing anything else */
    pid_t existing = check_existing_monitor();
    if (existing != 0)
    {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "ERROR:monitor already running with pid=%d\n", (int)existing);
        write_msg(buf);
        return EXIT_FAILURE;
    }

    if (install_handlers() != 0)
    {
        write_msg("ERROR:failed to install signal handlers\n");
        return EXIT_FAILURE;
    }

    sigset_t blocked, prev;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGINT);
    sigaddset(&blocked, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &blocked, &prev) == -1)
    {
        write_msg("ERROR:sigprocmask failed\n");
        return EXIT_FAILURE;
    }

    if (write_pid_file() != 0)
    {
        write_msg("ERROR:failed to write PID file\n");
        return EXIT_FAILURE;
    }

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg),
             "INFO:monitor started with pid=%d\n", (int)getpid());
    write_msg(start_msg);

    while (!stop_flag)
        sigsuspend(&prev);

    if (unlink(PID_FILE_PATH) == -1 && errno != ENOENT)
        perror("monitor_reports: unlink .monitor_pid failed");

    return EXIT_SUCCESS;
}
