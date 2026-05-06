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
 */

#define PID_FILE_PATH "data/.monitor_pid"

static volatile sig_atomic_t stop_flag = 0;

// on_sigint - shutdown handler. Only async-signal-safe calls allowed here.
static void on_sigint(int signo)
{
    (void)signo;
    static const char msg[] = "\nmonitor_reports: SIGINT received, shutting down\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    stop_flag = 1;
}

// on_sigusr1 - new-report notification handler.
static void on_sigusr1(int signo)
{
    (void)signo;
    static const char msg[] = "monitor_reports: new report added\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

// install_handlers - register SIGINT and SIGUSR1 with sigaction, with each
// handler masking the other so they cannot interleave.
static int install_handlers(void)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGUSR1);
    sa.sa_flags = 0;

    sa.sa_handler = on_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("monitor_reports: sigaction(SIGINT)");
        return -1;
    }

    sa.sa_handler = on_sigusr1;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("monitor_reports: sigaction(SIGUSR1)");
        return -1;
    }

    return 0;
}

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
    if (install_handlers() != 0)
        return EXIT_FAILURE;

    // Block SIGINT and SIGUSR1 in the main thread. sigsuspend() will
    // atomically unblock them, wait for one to be delivered, then
    // re-block on return — this closes the race window between checking
    // stop_flag and waiting for the next signal.
    sigset_t blocked, prev;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGINT);
    sigaddset(&blocked, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &blocked, &prev) == -1)
    {
        perror("monitor_reports: sigprocmask");
        return EXIT_FAILURE;
    }

    if (write_pid_file() != 0)
        return EXIT_FAILURE;

    const char *msg = "monitor_reports: started, waiting for signals\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    while (!stop_flag)
        sigsuspend(&prev);

    if (unlink(PID_FILE_PATH) == -1 && errno != ENOENT)
        perror("monitor_reports: unlink .monitor_pid failed");

    return EXIT_SUCCESS;
}
