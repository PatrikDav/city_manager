#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

/*
 * city_hub.c - Interactive command-line hub for the city manager system
 *
 * Commands:
 *   start_monitor                   - launch hub_mon background process
 *   calculate_scores <district ...> - compute per-inspector workload scores
 *   exit / quit                     - shut down city_hub
 */

#define MONITOR_BIN  "./monitor_reports"
#define MAX_LINE     1024
#define MAX_ARGS     64

/* PID of the currently running hub_mon child, or -1 if none */
static pid_t hub_mon_pid = -1;

/* ------------------------------------------------------------------ */
/* hub_mon logic — runs entirely inside the hub_mon child process      */
/* ------------------------------------------------------------------ */

/* Read a complete newline-terminated line from fd into buf (NUL-terminated,
 * newline stripped). Returns number of chars read, 0 on EOF, -1 on error. */
static ssize_t read_line(int fd, char *buf, size_t capacity)
{
    size_t len = 0;
    char   ch;
    ssize_t n;

    while (len < capacity - 1 && (n = read(fd, &ch, 1)) > 0)
    {
        if (ch == '\n')
            break;
        buf[len++] = ch;
    }

    if (n == -1)
        return -1;

    buf[len] = '\0';
    return (ssize_t)len;
}

/* Print a monitor message line received from the pipe in a human-friendly
 * format.  Lines are expected to be "TYPE:text", e.g. "INFO:monitor started". */
static void display_monitor_msg(const char *line)
{
    const char *colon = strchr(line, ':');
    if (!colon)
    {
        printf("\n[hub_mon] %s\n", line);
        fflush(stdout);
        return;
    }

    /* Split at the first colon */
    char type[32];
    size_t type_len = (size_t)(colon - line);
    if (type_len >= sizeof(type))
        type_len = sizeof(type) - 1;
    strncpy(type, line, type_len);
    type[type_len] = '\0';

    const char *message = colon + 1;

    if (strcmp(type, "ERROR") == 0)
        printf("\n[hub_mon] ERROR: %s\n", message);
    else if (strcmp(type, "NOTIFY") == 0)
        printf("\n[hub_mon] notification: %s\n", message);
    else
        printf("\n[hub_mon] %s\n", message);

    fflush(stdout);
}

/* Entry point for the hub_mon child process.
 *
 * 1. Create a pipe.
 * 2. Fork monitor_reports, redirecting its stdout to the pipe write-end.
 * 3. Read structured messages from the pipe and relay them to the user.
 * 4. When the pipe closes (monitor exited), report it and exit. */
static void hub_mon_run(void)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("hub_mon: pipe");
        _exit(EXIT_FAILURE);
    }

    pid_t monitor_pid = fork();
    if (monitor_pid == -1)
    {
        perror("hub_mon: fork");
        _exit(EXIT_FAILURE);
    }

    if (monitor_pid == 0)
    {
        /* ---- monitor child ---- */
        /* Redirect stdout → pipe write-end so all monitor output reaches hub_mon */
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("hub_mon: dup2");
            _exit(EXIT_FAILURE);
        }
        close(pipefd[0]);
        close(pipefd[1]);

        char *args[] = {MONITOR_BIN, NULL};
        execvp(MONITOR_BIN, args);
        /* exec only returns on failure */
        perror("hub_mon: execvp monitor_reports");
        _exit(EXIT_FAILURE);
    }

    /* ---- hub_mon parent ---- */
    close(pipefd[1]); /* we only read */

    char line[MAX_LINE];
    ssize_t n;

    /* Relay every line from the monitor until the pipe closes */
    while ((n = read_line(pipefd[0], line, sizeof(line))) > 0)
        display_monitor_msg(line);

    close(pipefd[0]);

    /* Reap the monitor child */
    int status;
    waitpid(monitor_pid, &status, 0);

    printf("\n[hub_mon] Monitor process has ended.\n");
    fflush(stdout);
    _exit(EXIT_SUCCESS);
}

/* ------------------------------------------------------------------ */
/* city_hub commands                                                   */
/* ------------------------------------------------------------------ */

static void cmd_start_monitor(void)
{
    /* Check if a hub_mon is already running */
    if (hub_mon_pid != -1 && kill(hub_mon_pid, 0) == 0)
    {
        printf("Monitor already running (hub_mon pid=%d).\n", (int)hub_mon_pid);
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("city_hub: fork hub_mon");
        return;
    }

    if (pid == 0)
    {
        /* ---- hub_mon child ---- */
        hub_mon_run(); /* never returns */
        _exit(EXIT_FAILURE);
    }

    /* ---- city_hub parent ---- */
    hub_mon_pid = pid;
    printf("Monitor hub started (hub_mon pid=%d).\n", (int)pid);
}

/* ------------------------------------------------------------------ */
/* Argument tokeniser                                                  */
/* ------------------------------------------------------------------ */

/* Split a mutable string into tokens on whitespace.
 * Returns number of tokens stored in argv (argv is NULL-terminated). */
static int tokenise(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;

    while (*p != '\0' && argc < max_args - 1)
    {
        /* skip leading whitespace */
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\0')
            break;

        argv[argc++] = p;

        /* advance to next whitespace or end */
        while (*p != ' ' && *p != '\t' && *p != '\0')
            p++;
        if (*p != '\0')
            *p++ = '\0';
    }

    argv[argc] = NULL;
    return argc;
}

/* ------------------------------------------------------------------ */
/* Main interactive loop                                               */
/* ------------------------------------------------------------------ */

int main(void)
{
    /* Auto-reap background children so hub_mon never becomes a zombie
     * when city_hub is still running after hub_mon exits. */
    signal(SIGCHLD, SIG_IGN);

    printf("city_hub started. Type 'exit' to quit.\n");

    char line[MAX_LINE];

    while (1)
    {
        printf("city_hub> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        /* Strip trailing newline */
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0')
            continue;

        char *argv[MAX_ARGS];
        int   argc = tokenise(line, argv, MAX_ARGS);
        if (argc == 0)
            continue;

        const char *cmd = argv[0];

        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0)
        {
            printf("Goodbye.\n");
            break;
        }
        else if (strcmp(cmd, "start_monitor") == 0)
        {
            cmd_start_monitor();
        }
        else if (strcmp(cmd, "calculate_scores") == 0)
        {
            printf("calculate_scores: not yet implemented.\n");
        }
        else
        {
            printf("Unknown command: %s\n", cmd);
            printf("Available commands: start_monitor, calculate_scores, exit\n");
        }
    }

    return EXIT_SUCCESS;
}
