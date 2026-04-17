#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "filter.h"
#include "permissions.h"
#include "logger.h"
#include "report_io.h"

/*
 * filter.c - Report Filtering
 *
 * parse_condition and match_condition were generated with AI assistance.
 * See ai_usage.md for prompts, generated output, and changes made.
 */

// parse_condition (AI-assisted)
// Splits "severity:>=:2" into field="severity", op=">=", value="2"
// Strategy: find the first ':', copy field; find the second ':', copy op; copy the rest as value
int parse_condition(const char *input, char *field, char *op, char *value)
{
    const char *first_colon = strchr(input, ':');
    if (first_colon == NULL)
        return -1;

    // Copy everything before the first colon into field
    size_t field_len = (size_t)(first_colon - input);
    strncpy(field, input, field_len);
    field[field_len] = '\0';

    const char *second_colon = strchr(first_colon + 1, ':');
    if (second_colon == NULL)
        return -1;

    // Copy what is between the two colons into op
    size_t op_len = (size_t)(second_colon - (first_colon + 1));
    strncpy(op, first_colon + 1, op_len);
    op[op_len] = '\0';

    // Copy everything after the second colon into value
    strncpy(value, second_colon + 1, 255);
    value[255] = '\0';

    return 0;
}

// compare_int - helper to apply an operator to two integers
static int compare_int(int a, const char *op, int b)
{
    if (strcmp(op, "==") == 0) return a == b;
    if (strcmp(op, "!=") == 0) return a != b;
    if (strcmp(op, "<")  == 0) return a <  b;
    if (strcmp(op, "<=") == 0) return a <= b;
    if (strcmp(op, ">")  == 0) return a >  b;
    if (strcmp(op, ">=") == 0) return a >= b;
    return 0;
}

// compare_str - helper to apply an operator to strcmp result
static int compare_str(const char *a, const char *op, const char *b)
{
    int cmp = strcmp(a, b);
    if (strcmp(op, "==") == 0) return cmp == 0;
    if (strcmp(op, "!=") == 0) return cmp != 0;
    if (strcmp(op, "<")  == 0) return cmp <  0;
    if (strcmp(op, "<=") == 0) return cmp <= 0;
    if (strcmp(op, ">")  == 0) return cmp >  0;
    if (strcmp(op, ">=") == 0) return cmp >= 0;
    return 0;
}

// match_condition (AI-assisted)
// Checks one field of a report against a condition.
// Numeric fields (severity, timestamp) use integer comparison.
// String fields (category, inspector) use strcmp-based comparison.
int match_condition(const Report *r, const char *field, const char *op, const char *value)
{
    if (strcmp(field, "severity") == 0)
        return compare_int(r->severity, op, atoi(value));

    if (strcmp(field, "category") == 0)
        return compare_str(r->category, op, value);

    if (strcmp(field, "inspector") == 0)
        return compare_str(r->inspector_name, op, value);

    if (strcmp(field, "timestamp") == 0)
        return compare_int((int)r->timestamp, op, atoi(value));

    fprintf(stderr, "Warning: unknown filter field '%s'\n", field);
    return 0;
}

// filter_reports
int filter_reports(const char *district, const Args *args)
{
    char path[256];
    snprintf(path, sizeof(path), "data/%s/reports.dat", district);

    if (check_role_access(path, args->role, 0) != 0)
        return -1;

    // Parse all condition strings into Condition structs
    Condition conds[MAX_CONDITIONS];
    int cond_count = 0;

    for (int i = 0; i < args->filter_count && i < MAX_CONDITIONS; i++)
    {
        Condition c;
        if (parse_condition(args->filter_conditions[i], c.field, c.op, c.value) == 0)
            conds[cond_count++] = c;
        else
            fprintf(stderr, "Warning: skipping invalid condition '%s'\n",
                    args->filter_conditions[i]);
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("filter_reports: open failed");
        return -1;
    }

    // Read records one by one; print those that satisfy ALL conditions
    Report r;
    int matched = 0;
    while (read_full(fd, &r, sizeof(Report)) == (ssize_t)sizeof(Report))
    {
        int pass = 1;
        for (int i = 0; i < cond_count; i++)
        {
            if (!match_condition(&r, conds[i].field, conds[i].op, conds[i].value))
            {
                pass = 0;
                break;
            }
        }

        if (!pass)
            continue;

        char ts_str[32];
        struct tm *ts = localtime(&r.timestamp);
        strftime(ts_str, sizeof(ts_str), "%Y-%m-%d %H:%M", ts);

        printf("ID: %-4d | Inspector: %-20s | Category: %-12s | Severity: %d | %s\n",
               r.report_id, r.inspector_name, r.category, r.severity, ts_str);
        printf("         GPS: %.6f, %.6f\n", r.gps_lat, r.gps_lon);
        printf("         Desc: %s\n\n", r.description);
        matched++;
    }

    close(fd);

    printf("%d record(s) matched.\n", matched);

    const char *role_str = (args->role == ROLE_MANAGER) ? "manager" : "inspector";
    log_operation(district, role_str, args->username, "FILTER");

    return 0;
}
