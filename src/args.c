#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "args.h"

/*
 * args.c - Args Parsing Implementation
 *
 * This file implements the parse_args() function declared in args.h.
 *
 */

int parse_args(int argc, char *argv[], Args *args)
{
    // initialize all fileds
    memset(args, 0, sizeof(Args));
    args->command = CMD_UNKNOWN;
    args->report_id = -1;
    args->threshold_value = -1;

    // Check if role and user has been provied
    int has_role = 0;
    int has_user = 0;

    // Need at least: program_name --role X --user Y --command district = 7 args minimum
    if (argc < 7)
    {
        fprintf(stderr, "Error: not enough arguments (minimum 7 required, got %d)\n", argc);
        return -1;
    }

    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "--role") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: --role requires a value (inspector or manager)\n");
                return -1;
            }
            if (strcmp(argv[i + 1], "inspector") == 0)
            {
                args->role = ROLE_INSPECTOR;
            }
            else if (strcmp(argv[i + 1], "manager") == 0)
            {
                args->role = ROLE_MANAGER;
            }
            else
            {
                fprintf(stderr, "Error: unknown role '%s' (must be 'inspector' or 'manager')\n",
                        argv[i + 1]);
                return -1;
            }
            has_role = 1;
            i += 2;
        }
        else if (strcmp(argv[i], "--user") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: --user requires a name\n");
                return -1;
            }
            strncpy(args->username, argv[i + 1], NAME_LEN - 1);
            args->username[NAME_LEN - 1] = '\0';
            has_user = 1;
            i += 2;
        }
        else if (strcmp(argv[i], "--add") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: --add requires a district name\n");
                return -1;
            }
            args->command = CMD_ADD;
            strncpy(args->district, argv[i + 1], NAME_LEN - 1);
            args->district[NAME_LEN - 1] = '\0';
            i += 2;
        }
        else if (strcmp(argv[i], "--list") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: --list requires a district name\n");
                return -1;
            }
            args->command = CMD_LIST;
            strncpy(args->district, argv[i + 1], NAME_LEN - 1);
            args->district[NAME_LEN - 1] = '\0';
            i += 2;
        }
        else if (strcmp(argv[i], "--view") == 0)
        {
            if (i + 2 >= argc)
            {
                fprintf(stderr, "Error: --view requires <district> <report_id>\n");
                return -1;
            }
            args->command = CMD_VIEW;
            strncpy(args->district, argv[i + 1], NAME_LEN - 1);
            args->district[NAME_LEN - 1] = '\0';
            args->report_id = atoi(argv[i + 2]);
            if (args->report_id <= 0)
            {
                fprintf(stderr, "Error: report_id must be a positive integer, got '%s'\n",
                        argv[i + 2]);
                return -1;
            }
            i += 3;
        }
        else if (strcmp(argv[i], "--remove_report") == 0)
        {
            if (i + 2 >= argc)
            {
                fprintf(stderr, "Error: --remove_report requires <district> <report_id>\n");
                return -1;
            }
            args->command = CMD_REMOVE_REPORT;
            strncpy(args->district, argv[i + 1], NAME_LEN - 1);
            args->district[NAME_LEN - 1] = '\0';
            args->report_id = atoi(argv[i + 2]);
            if (args->report_id <= 0)
            {
                fprintf(stderr, "Error: report_id must be a positive integer, got '%s'\n",
                        argv[i + 2]);
                return -1;
            }
            i += 3;
        }
        else if (strcmp(argv[i], "--update_threshold") == 0)
        {
            if (i + 2 >= argc)
            {
                fprintf(stderr, "Error: --update_threshold requires <district> <value>\n");
                return -1;
            }
            args->command = CMD_UPDATE_THRESHOLD;
            strncpy(args->district, argv[i + 1], NAME_LEN - 1);
            args->district[NAME_LEN - 1] = '\0';
            args->threshold_value = atoi(argv[i + 2]);
            i += 3;
        }
        else if (strcmp(argv[i], "--remove_district") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: --remove_district requires a district name\n");
                return -1;
            }
            args->command = CMD_REMOVE_DISTRICT;
            strncpy(args->district, argv[i + 1], NAME_LEN - 1);
            args->district[NAME_LEN - 1] = '\0';
            i += 2;
        }
        else if (strcmp(argv[i], "--filter") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: --filter requires <district> [conditions...]\n");
                return -1;
            }
            args->command = CMD_FILTER;
            strncpy(args->district, argv[i + 1], NAME_LEN - 1);
            args->district[NAME_LEN - 1] = '\0';

            args->filter_conditions = &argv[i + 2];
            args->filter_count = argc - (i + 2);

            i = argc;
        }
        else
        {
            fprintf(stderr, "Error: unknown argument '%s'\n", argv[i]);
            return -1;
        }
    }

    // Final check
    if (!has_role)
    {
        fprintf(stderr, "Error: --role is required\n");
        return -1;
    }
    if (!has_user)
    {
        fprintf(stderr, "Error: --user is required\n");
        return -1;
    }
    if (args->command == CMD_UNKNOWN)
    {
        fprintf(stderr, "Error: no command specified (--add, --list, --view, etc.)\n");
        return -1;
    }

    return 0;
}

// help message with all valid command formats. Called on failer

void print_usage(const char *program_name)
{
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "  %s --role <inspector|manager> --user <name> <command>\n\n", program_name);
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  --add <district>                        Add a new report\n");
    fprintf(stderr, "  --list <district>                       List all reports\n");
    fprintf(stderr, "  --view <district> <report_id>           View a specific report\n");
    fprintf(stderr, "  --remove_report <district> <report_id>  Remove a report (manager only)\n");
    fprintf(stderr, "  --update_threshold <district> <value>   Update severity threshold (manager only)\n");
    fprintf(stderr, "  --filter <district> <conditions...>     Filter reports by conditions\n");
    fprintf(stderr, "  --remove_district <district>            Delete the district directory (manager only)\n");
    fprintf(stderr, "\nFilter condition format: field:operator:value\n");
    fprintf(stderr, "  Fields:    severity, category, inspector, timestamp\n");
    fprintf(stderr, "  Operators: ==, !=, <, <=, >, >=\n");
    fprintf(stderr, "  Example:   severity:>=:2 category:==:road\n");
}
