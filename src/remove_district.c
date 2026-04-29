#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "remove_district.h"

/*
 * remove_district.c - Delete an entire district directory
 */

// is_safe_district_name
static int is_safe_district_name(const char *name)
{
    if (name == NULL || name[0] == '\0')
        return 0;
    if (name[0] == '.')
        return 0;
    if (strchr(name, '/') != NULL)
        return 0;
    if (strstr(name, "..") != NULL)
        return 0;
    return 1;
}

int remove_district(const char *district, const Args *args)
{
    if (args->role != ROLE_MANAGER)
    {
        fprintf(stderr, "Error: only managers can remove a district\n");
        return -1;
    }

    if (!is_safe_district_name(district))
    {
        fprintf(stderr, "Error: refusing to remove district with unsafe name '%s'\n",
                district);
        return -1;
    }

    char path[256];
    snprintf(path, sizeof(path), "data/%s", district);

    struct stat sb;
    if (stat(path, &sb) == -1)
    {
        fprintf(stderr, "Error: district '%s' does not exist (%s)\n",
                district, strerror(errno));
        return -1;
    }
    if (!S_ISDIR(sb.st_mode))
    {
        fprintf(stderr, "Error: '%s' is not a directory\n", path);
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("remove_district: fork failed");
        return -1;
    }

    if (pid == 0)
    {
        char *argv[] = {"rm", "-rf", path, NULL};
        execvp("rm", argv);
        perror("remove_district: execvp failed");
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("remove_district: waitpid failed");
        return -1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        fprintf(stderr, "Error: rm -rf %s did not finish cleanly (status=%d)\n",
                path, status);
        return -1;
    }

    char link_path[256];
    snprintf(link_path, sizeof(link_path), "active_reports-%s", district);
    if (unlink(link_path) == -1 && errno != ENOENT)
        perror("remove_district: unlink symlink failed");

    printf("District '%s' removed (data/%s and active_reports-%s)\n",
           district, district, district);
    return 0;
}
