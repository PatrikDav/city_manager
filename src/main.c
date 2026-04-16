#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "report.h"

/*
 * main.c - Entry Point for city_manager
 */

int main(int argc, char *argv[])
{
    Args args;
    if (parse_args(argc, argv, &args) != 0)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    switch (args.command)
    {
    case CMD_ADD:
        printf("[STUB] Would execute ADD on district '%s' "
               "as %s '%s'\n",
               args.district,
               args.role == ROLE_MANAGER ? "manager" : "inspector",
               args.username);
        break;

    case CMD_LIST:
        printf("[STUB] Would execute LIST on district '%s' "
               "as %s '%s'\n",
               args.district,
               args.role == ROLE_MANAGER ? "manager" : "inspector",
               args.username);
        break;

    case CMD_VIEW:
        printf("[STUB] Would execute VIEW on district '%s', "
               "report_id=%d as %s '%s'\n",
               args.district, args.report_id,
               args.role == ROLE_MANAGER ? "manager" : "inspector",
               args.username);
        break;

    case CMD_REMOVE_REPORT:
        printf("[STUB] Would execute REMOVE_REPORT on district '%s', "
               "report_id=%d as %s '%s'\n",
               args.district, args.report_id,
               args.role == ROLE_MANAGER ? "manager" : "inspector",
               args.username);
        break;

    case CMD_UPDATE_THRESHOLD:
        printf("[STUB] Would execute UPDATE_THRESHOLD on district '%s', "
               "value=%d as %s '%s'\n",
               args.district, args.threshold_value,
               args.role == ROLE_MANAGER ? "manager" : "inspector",
               args.username);
        break;

    case CMD_FILTER:
        printf("[STUB] Would execute FILTER on district '%s' "
               "with %d condition(s) as %s '%s'\n",
               args.district, args.filter_count,
               args.role == ROLE_MANAGER ? "manager" : "inspector",
               args.username);
        break;

    case CMD_UNKNOWN:
        fprintf(stderr, "Error: unknown command\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
