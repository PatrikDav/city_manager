/*
 * permissions.h - Permission Checking and Enforcement
 * This module handles all Unix permission logic for city_manager.
 */

#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/stat.h>
#include "args.h"

#define PERM_DISTRICT_DIR 0750 /* rwxr-x--- : manager=rwx, inspector=r-x */
#define PERM_REPORTS_DAT 0664  /* rw-rw-r-- : manager=rw-, inspector=rw- */
#define PERM_DISTRICT_CFG 0640 /* rw-r----- : manager=rw-, inspector=r-- */
#define PERM_LOGGED 0644       /* rw-r--r-- : manager=rw-, inspector=r-- */

// set_permissions - Apply permission bits to a file or directory
void set_permissions(const char *path, mode_t mode);

// check_permission - Verify that specific permission bits are set on a file
int check_permission(const char *path, mode_t required_bits);

// check_role_access - Enforce role-based access before an operation
int check_role_access(const char *path, Role role, int need_write);

// format_permission_string - Convert a mode_t into a 9-character symbolic string
void format_permission_string(mode_t mode, char out[10]);

#endif /* PERMISSIONS_H */
