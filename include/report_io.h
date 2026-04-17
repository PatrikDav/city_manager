#ifndef REPORT_IO_H
#define REPORT_IO_H
#include <unistd.h>
#include "args.h"
#include "report.h"

/*
 * report_io.h
 */

// add_report - Prompt for fields interactively and append a new record
int add_report(const char *district, const Args *args);

// list_reports - Print all records + file metadata (size, perms, mtime)
int list_reports(const char *district, const Args *args);

// read_full - Read exactly count bytes, looping on partial reads
ssize_t read_full(int fd, void *buf, size_t count);

#endif /* REPORT_IO_H */
