#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "report_io.h"
#include "permissions.h"
#include "logger.h"
#include "symlinks.h"
#include "monitor_notify.h"

/*
 * report_io.c - Report CRUD operations
 *
 */

// read_full - loop until all count bytes are read
ssize_t read_full(int fd, void *buf, size_t count)
{
    size_t total = 0;
    while (total < count)
    {
        ssize_t n = read(fd, (char *)buf + total, count - total);
        if (n < 0)
            return -1;
        if (n == 0)
            break;
        total += (size_t)n;
    }
    return (ssize_t)total;
}

// read_line - read one line from fd into buf
static int read_line(int fd, char *buf, int max)
{
    int i = 0;
    char c;
    while (i < max - 1)
    {
        if (read(fd, &c, 1) <= 0 || c == '\n')
            break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

// add_report
int add_report(const char *district, const Args *args)
{
    char path[256];
    snprintf(path, sizeof(path), "data/%s/reports.dat", district);

    if (check_role_access(path, args->role, 1) != 0)
        return -1;

    int fd = open(path, O_RDWR);
    if (fd == -1)
    {
        perror("add_report: open failed");
        return -1;
    }

    // Scan all existing records to find the highest report_id
    Report r;
    int max_id = 0;
    while (read_full(fd, &r, sizeof(Report)) == (ssize_t)sizeof(Report))
    {
        if (r.report_id > max_id)
            max_id = r.report_id;
    }

    // Build the new report
    Report report;
    memset(&report, 0, sizeof(Report));
    report.report_id = max_id + 1;
    strncpy(report.inspector_name, args->username, NAME_LEN - 1);
    report.timestamp = time(NULL);

    // Prompt user for each field
    char buf[256];
    const char *prompt;

    prompt = "Category (road/lighting/flooding/...): ";
    write(STDOUT_FILENO, prompt, strlen(prompt));
    read_line(STDIN_FILENO, buf, sizeof(buf));
    strncpy(report.category, buf, CATEGORY_LEN - 1);

    prompt = "GPS Latitude: ";
    write(STDOUT_FILENO, prompt, strlen(prompt));
    read_line(STDIN_FILENO, buf, sizeof(buf));
    report.gps_lat = (float)atof(buf);

    prompt = "GPS Longitude: ";
    write(STDOUT_FILENO, prompt, strlen(prompt));
    read_line(STDIN_FILENO, buf, sizeof(buf));
    report.gps_lon = (float)atof(buf);

    prompt = "Severity (1=minor, 2=moderate, 3=critical): ";
    write(STDOUT_FILENO, prompt, strlen(prompt));
    read_line(STDIN_FILENO, buf, sizeof(buf));
    report.severity = atoi(buf);

    prompt = "Description: ";
    write(STDOUT_FILENO, prompt, strlen(prompt));
    read_line(STDIN_FILENO, buf, sizeof(buf));
    strncpy(report.description, buf, DESC_LEN - 1);

    // Seek to end and append the record
    lseek(fd, 0, SEEK_END);
    if (write(fd, &report, sizeof(Report)) != (ssize_t)sizeof(Report))
    {
        perror("add_report: write failed");
        close(fd);
        return -1;
    }

    close(fd);

    printf("Report #%d added to district '%s'\n", report.report_id, district);

    // Log the operation
    const char *role_str = (args->role == ROLE_MANAGER) ? "manager" : "inspector";
    char op[64];
    snprintf(op, sizeof(op), "ADD id=%d", report.report_id);
    log_operation(district, role_str, args->username, op);

    // Notify the monitor. notify_monitor()
    char monitor_msg[128];
    notify_monitor(monitor_msg, sizeof(monitor_msg));
    log_operation(district, role_str, args->username, monitor_msg);

    return 0;
}

// list_reports
int list_reports(const char *district, const Args *args)
{
    char path[256];
    snprintf(path, sizeof(path), "data/%s/reports.dat", district);

    // Both roles can read reports.dat
    if (check_role_access(path, args->role, 0) != 0)
        return -1;

    struct stat sb;
    if (stat(path, &sb) == -1)
    {
        perror("list_reports: stat failed");
        return -1;
    }

    // Print the file info header line
    char perm_str[10];
    format_permission_string(sb.st_mode, perm_str);

    char mtime_str[32];
    struct tm *tm_info = localtime(&sb.st_mtime);
    strftime(mtime_str, sizeof(mtime_str), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("File: reports.dat | size: %lld bytes | permissions: %s | modified: %s\n",
           (long long)sb.st_size, perm_str, mtime_str);

    // Show symlink status using lstat() to inspect the link itself
    char link_path[256];
    char link_target[256];
    snprintf(link_path, sizeof(link_path), "active_reports-%s", district);

    // readlink()
    ssize_t rlen = readlink(link_path, link_target, sizeof(link_target) - 1);
    if (rlen != -1)
    {
        link_target[rlen] = '\0';
        int status = resolve_or_warn(link_path);
        printf("Symlink: %s -> %s [%s]\n",
               link_path, link_target, status == 0 ? "OK" : "DANGLING");
    }
    else
    {
        printf("Symlink: active_reports-%s [not found]\n", district);
    }

    printf("------------------------------------------------------------\n");

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("list_reports: open failed");
        return -1;
    }

    // Read and print records one by one
    Report r;
    int count = 0;
    while (read_full(fd, &r, sizeof(Report)) == (ssize_t)sizeof(Report))
    {
        char ts_str[32];
        struct tm *ts = localtime(&r.timestamp);
        strftime(ts_str, sizeof(ts_str), "%Y-%m-%d %H:%M", ts);

        printf("ID: %-4d | Inspector: %-20s | Category: %-12s | Severity: %d | %s\n",
               r.report_id, r.inspector_name, r.category, r.severity, ts_str);
        printf("         GPS: %.6f, %.6f\n", r.gps_lat, r.gps_lon);
        printf("         Desc: %s\n\n", r.description);
        count++;
    }

    close(fd);

    if (count == 0)
        printf("No reports in district '%s'\n", district);
    else
        printf("%d report(s) total.\n", count);

    const char *role_str = (args->role == ROLE_MANAGER) ? "manager" : "inspector";
    log_operation(district, role_str, args->username, "LIST");

    return 0;
}

// view_report
int view_report(const char *district, const Args *args)
{
    char path[256];
    snprintf(path, sizeof(path), "data/%s/reports.dat", district);

    if (check_role_access(path, args->role, 0) != 0)
        return -1;

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("view_report: open failed");
        return -1;
    }

    Report r;
    int found = 0;
    while (read_full(fd, &r, sizeof(Report)) == (ssize_t)sizeof(Report))
    {
        if (r.report_id != args->report_id)
            continue;

        char ts_str[32];
        struct tm *ts = localtime(&r.timestamp);
        strftime(ts_str, sizeof(ts_str), "%Y-%m-%d %H:%M:%S", ts);

        printf("------------------------------------------------------------\n");
        printf("Report ID   : %d\n", r.report_id);
        printf("Inspector   : %s\n", r.inspector_name);
        printf("Category    : %s\n", r.category);
        printf("Severity    : %d (%s)\n", r.severity,
               r.severity == 1 ? "minor" : r.severity == 2 ? "moderate"
                                                           : "critical");
        printf("GPS         : %.6f, %.6f\n", r.gps_lat, r.gps_lon);
        printf("Timestamp   : %s\n", ts_str);
        printf("Description : %s\n", r.description);
        printf("------------------------------------------------------------\n");
        found = 1;
        break;
    }

    close(fd);

    if (!found)
    {
        fprintf(stderr, "Error: report #%d not found in '%s'\n", args->report_id, district);
        return -1;
    }

    const char *role_str = (args->role == ROLE_MANAGER) ? "manager" : "inspector";
    char op[64];
    snprintf(op, sizeof(op), "VIEW id=%d", args->report_id);
    log_operation(district, role_str, args->username, op);

    return 0;
}

// remove_report
int remove_report(const char *district, const Args *args)
{
    char path[256];
    snprintf(path, sizeof(path), "data/%s/reports.dat", district);

    // Manager-only operation
    if (args->role != ROLE_MANAGER)
    {
        fprintf(stderr, "Error: only managers can remove reports\n");
        return -1;
    }

    if (check_role_access(path, args->role, 1) != 0)
        return -1;

    int fd = open(path, O_RDWR);
    if (fd == -1)
    {
        perror("remove_report: open failed");
        return -1;
    }

    // Get total file size and derive number of records
    off_t file_size = lseek(fd, 0, SEEK_END);
    int count = (int)(file_size / (off_t)sizeof(Report));

    if (count == 0)
    {
        fprintf(stderr, "Error: no reports in district '%s'\n", district);
        close(fd);
        return -1;
    }

    int target_index = -1;
    Report r;
    for (int i = 0; i < count; i++)
    {
        lseek(fd, (off_t)i * (off_t)sizeof(Report), SEEK_SET);
        if (read_full(fd, &r, sizeof(Report)) != (ssize_t)sizeof(Report))
            break;
        if (r.report_id == args->report_id)
        {
            target_index = i;
            break;
        }
    }

    if (target_index == -1)
    {
        fprintf(stderr, "Error: report #%d not found in '%s'\n", args->report_id, district);
        close(fd);
        return -1;
    }

    Report temp;
    for (int i = target_index + 1; i < count; i++)
    {
        lseek(fd, (off_t)i * (off_t)sizeof(Report), SEEK_SET);
        read_full(fd, &temp, sizeof(Report));

        lseek(fd, (off_t)(i - 1) * (off_t)sizeof(Report), SEEK_SET);
        write(fd, &temp, sizeof(Report));
    }

    ftruncate(fd, file_size - (off_t)sizeof(Report));
    close(fd);

    printf("Report #%d removed from district '%s'\n", args->report_id, district);

    char op[64];
    snprintf(op, sizeof(op), "REMOVE id=%d", args->report_id);
    log_operation(district, "manager", args->username, op);

    return 0;
}

// update_threshold
int update_threshold(const char *district, const Args *args)
{
    char path[256];
    snprintf(path, sizeof(path), "data/%s/district.cfg", district);

    // Manager only
    if (args->role != ROLE_MANAGER)
    {
        fprintf(stderr, "Error: only managers can update the threshold\n");
        return -1;
    }

    // Verify district.cfg still has exactly 0640 permissions
    // If someone changed them externally the spec requires we refuse and report it
    struct stat sb;
    if (stat(path, &sb) == -1)
    {
        perror("update_threshold: stat failed");
        return -1;
    }

    if ((sb.st_mode & 0777) != PERM_DISTRICT_CFG)
    {
        fprintf(stderr,
                "Error: district.cfg permissions are %04o, expected %04o — refusing write\n",
                sb.st_mode & 0777, PERM_DISTRICT_CFG);
        return -1;
    }

    // Open and overwrite (O_TRUNC resets the file to 0 bytes before writing)
    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1)
    {
        perror("update_threshold: open failed");
        return -1;
    }

    char content[64];
    ssize_t len = snprintf(content, sizeof(content), "threshold=%d\n", args->threshold_value);
    if (write(fd, content, (size_t)len) != len)
    {
        perror("update_threshold: write failed");
        close(fd);
        return -1;
    }

    close(fd);

    printf("Threshold updated to %d in district '%s'\n", args->threshold_value, district);

    char logop[64];
    snprintf(logop, sizeof(logop), "UPDATE_THRESHOLD value=%d", args->threshold_value);
    log_operation(district, "manager", args->username, logop);

    return 0;
}
