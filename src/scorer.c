#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "report.h"

/*
 * scorer.c - Workload score calculator for a single district
 *
 * Usage: scorer <district_name>
 *
 * Reads data/<district>/reports.dat, computes the sum of severity levels
 * for each inspector, and prints a plain-text summary to stdout.
 * city_hub redirects stdout to a pipe via dup2() before exec'ing this program.
 */

#define MAX_INSPECTORS 256
#define DATA_PREFIX    "data/"
#define REPORTS_FILE   "/reports.dat"

typedef struct
{
    char name[NAME_LEN];
    int  score;
} InspectorScore;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "scorer: usage: scorer <district_name>\n");
        return EXIT_FAILURE;
    }

    const char *district = argv[1];

    /* Validate district name: no path traversal */
    if (strstr(district, "..") || strchr(district, '/') || district[0] == '.')
    {
        fprintf(stderr, "scorer: invalid district name: %s\n", district);
        return EXIT_FAILURE;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s%s%s", DATA_PREFIX, district, REPORTS_FILE);

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        printf("District: %s\n  [error: cannot open reports file: %s]\n",
               district, strerror(errno));
        return EXIT_FAILURE;
    }

    InspectorScore scores[MAX_INSPECTORS];
    int count = 0;

    Report r;
    ssize_t n;
    while ((n = read(fd, &r, sizeof(r))) == (ssize_t)sizeof(r))
    {
        int found = 0;
        for (int i = 0; i < count; i++)
        {
            if (strncmp(scores[i].name, r.inspector_name, NAME_LEN) == 0)
            {
                scores[i].score += r.severity;
                found = 1;
                break;
            }
        }
        if (!found && count < MAX_INSPECTORS)
        {
            strncpy(scores[count].name, r.inspector_name, NAME_LEN - 1);
            scores[count].name[NAME_LEN - 1] = '\0';
            scores[count].score = r.severity;
            count++;
        }
    }
    close(fd);

    printf("District: %s\n", district);
    if (count == 0)
        printf("  (no reports found)\n");
    else
        for (int i = 0; i < count; i++)
            printf("  Inspector %-30s score: %d\n", scores[i].name, scores[i].score);

    return EXIT_SUCCESS;
}
