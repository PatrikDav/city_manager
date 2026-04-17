#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "report.h"
#include "district.h"
#include "logger.h"
#include "report_io.h"

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

    if (ensure_district(args.district) != 0)
        return EXIT_FAILURE;

    const char *role_str = (args.role == ROLE_MANAGER) ? "manager" : "inspector";

    switch (args.command)
    {
    case CMD_ADD:
        add_report(args.district, &args);
        break;

    case CMD_LIST:
        list_reports(args.district, &args);
        break;

    case CMD_VIEW:
        view_report(args.district, &args);
        break;

    case CMD_REMOVE_REPORT:
        remove_report(args.district, &args);
        break;

    case CMD_UPDATE_THRESHOLD:
        printf("[STUB] Would execute UPDATE_THRESHOLD on district '%s', value=%d as %s '%s'\n",
               args.district, args.threshold_value, role_str, args.username);
        log_operation(args.district, role_str, args.username, "UPDATE_THRESHOLD");
        break;

    case CMD_FILTER:
        printf("[STUB] Would execute FILTER on district '%s' with %d condition(s) as %s '%s'\n",
               args.district, args.filter_count, role_str, args.username);
        log_operation(args.district, role_str, args.username, "FILTER");
        break;

    case CMD_UNKNOWN:
        fprintf(stderr, "Error: unknown command\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
