#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "symlinks.h"

/*
 * symlinks.c - Symbolic Link Management
 */

// create_district_symlink
int create_district_symlink(const char *district)
{
    char link_path[256];
    char target[256];

    snprintf(link_path, sizeof(link_path), "active_reports-%s", district);
    snprintf(target,    sizeof(target),    "data/%s/reports.dat", district);

    // symlink(target, linkpath) creates: linkpath -> target
    if (symlink(target, link_path) == -1)
    {
        // EEXIST means the symlink is already there — not an error
        if (errno != EEXIST)
        {
            perror("create_district_symlink: symlink failed");
            return -1;
        }
    }

    return 0;
}

// resolve_or_warn
int resolve_or_warn(const char *link_path)
{
    struct stat lst;

    // lstat() checks the link itself without following it
    if (lstat(link_path, &lst) == -1)
        return -1;

    // If it's not a symlink, we can't reason about it here
    if (!S_ISLNK(lst.st_mode))
        return -1;

    struct stat st;

    // stat() follows the link to the target
    // If it fails here, the target is gone -> dangling link
    if (stat(link_path, &st) == -1)
    {
        fprintf(stderr, "WARNING: symlink '%s' is dangling (target missing)\n", link_path);
        return 1;
    }

    return 0;
}
