#ifndef FILTER_H
#define FILTER_H
#include "report.h"
#include "args.h"

// filter.h - Report Filtering

#define MAX_CONDITIONS 8

typedef struct
{
    char field[32];
    char op[4];
    char value[256];
} Condition;

// parse_condition
int parse_condition(const char *input, char *field, char *op, char *value);

// match_condition
int match_condition(const Report *r, const char *field, const char *op, const char *value);

// filter_reports
int filter_reports(const char *district, const Args *args);

#endif /* FILTER_H */
