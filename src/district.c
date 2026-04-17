#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "district.h"
#include "permissions.h"

/*
 * district.c - District Directory Initialization
 */

// create_file_if_missing
static int create_file_if_missing(const char *path, mode_t mode, const char *content)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);

    if (fd == -1)
    {
        if (errno == EEXIST)
        {
            set_permissions(path, mode);
            return 0;
        }
        perror("create_file_if_missing: open failed");
        return -1;
    }

    if (content != NULL)
    {
        ssize_t len = (ssize_t)strlen(content);
        if (write(fd, content, (size_t)len) != len)
        {
            perror("create_file_if_missing: write failed");
            close(fd);
            return -1;
        }
    }

    close(fd);

    set_permissions(path, mode);
    return 0;
}

// ensure_district
int ensure_district(const char *name)
{
    char path[256];

    if (mkdir("data", 0750) == -1 && errno != EEXIST)
    {
        perror("ensure_district: cannot create data/");
        return -1;
    }

    snprintf(path, sizeof(path), "data/%s", name);
    if (mkdir(path, 0750) == -1 && errno != EEXIST)
    {
        fprintf(stderr, "ensure_district: cannot create '%s': ", path);
        perror("");
        return -1;
    }
    set_permissions(path, PERM_DISTRICT_DIR);

    snprintf(path, sizeof(path), "data/%s/reports.dat", name);
    if (create_file_if_missing(path, PERM_REPORTS_DAT, NULL) != 0)
        return -1;

    snprintf(path, sizeof(path), "data/%s/district.cfg", name);
    if (create_file_if_missing(path, PERM_DISTRICT_CFG, "threshold=2\n") != 0)
        return -1;

    snprintf(path, sizeof(path), "data/%s/logged_district", name);
    if (create_file_if_missing(path, PERM_LOGGED, NULL) != 0)
        return -1;

    return 0;
}
