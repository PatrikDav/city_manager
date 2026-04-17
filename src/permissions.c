#include <stdio.h>
#include <sys/stat.h>
#include "permissions.h"
#include "args.h"

/*
 * permissions.c - Permission Checking and Enforcement Implementation
 */

// set_permissions
void set_permissions(const char *path, mode_t mode)
{
    if (chmod(path, mode) == -1)
    {
        perror("set_permissions: chmod failed");
    }
}

// check_permission
int check_permission(const char *path, mode_t required_bits)
{
    struct stat sb;

    if (stat(path, &sb) == -1)
    {
        perror("check_permission: stat failed");
        return -1;
    }

    if ((sb.st_mode & required_bits) == required_bits)
        return 1;

    return 0;
}

// check_role_access
int check_role_access(const char *path, Role role, int need_write)
{
    mode_t required_bit;

    if (role == ROLE_MANAGER)
    {
        required_bit = need_write ? S_IWUSR : S_IRUSR;
    }
    else
    {
        required_bit = need_write ? S_IWGRP : S_IRGRP;
    }

    int result = check_permission(path, required_bit);

    if (result == 0)
    {
        fprintf(stderr,
                "Error: permission denied — %s does not have %s access to '%s'\n",
                role == ROLE_MANAGER ? "manager" : "inspector",
                need_write ? "write" : "read",
                path);
        return -1;
    }

    if (result == -1)
        return -1;

    return 0;
}

// format_permission_string
void format_permission_string(mode_t mode, char out[10])
{
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';
    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';
    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';
    out[9] = '\0';
}
