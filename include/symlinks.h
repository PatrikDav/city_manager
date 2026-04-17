/*
 * symlinks.h - Symbolic Link Management
 *
 * For every district, a symlink is created in the program's working directory:
 *   active_reports-<district>  ->  data/<district>/reports.dat
 */

#ifndef SYMLINKS_H
#define SYMLINKS_H

// create_district_symlink - Skips silently if the symlink already exists
int create_district_symlink(const char *district);

// resolve_or_warn - Check symlink target exists
int resolve_or_warn(const char *link_path);

#endif /* SYMLINKS_H */
